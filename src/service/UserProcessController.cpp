#include "service/UserProcessController.hpp"
#include "presentation/UserInterface.hpp"
#include "service/InventoryService.hpp"
#include "service/OrderService.hpp"
#include "service/PrepaymentService.hpp"
#include "service/MessageService.hpp"
#include "service/DistanceService.hpp"
#include "service/ErrorService.hpp"
#include "domain/drink.h"         
#include "domain/order.h"
#include "domain/vendingMachine.h"
#include "domain/prepaymentCode.h"
#include "network/message.hpp"
#include "network/PaymentCallbackReceiver.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <thread>   // std::this_thread::sleep_for (간단한 지연용)

#include <iostream>

namespace service {

/**
 * @brief UserProcessController 생성자.
 * 모든 의존성 객체들을 주입받고, 현재 자판기의 정보를 초기화합니다.
 * @param ui 사용자 인터페이스 객체에 대한 참조.
 * @param inventoryService 재고 관련 서비스 객체에 대한 참조.
 * @param orderService 주문 관련 서비스 객체에 대한 참조.
 * @param prepaymentService 선결제 관련 서비스 객체에 대한 참조.
 * @param messageService 메시지 송수신 서비스 객체에 대한 참조.
 * @param distanceService 거리 계산 서비스 객체에 대한 참조.
 * @param errorService 오류 처리 서비스 객체에 대한 참조.
 * @param ioContext Boost.Asio io_context 객체에 대한 참조.
 * @param myVmId 현재 자판기의 고유 ID.
 * @param myVmX 현재 자판기의 X 좌표.
 * @param myVmY 현재 자판기의 Y 좌표.
 */
UserProcessController::UserProcessController(
    presentation::UserInterface& ui,
    service::InventoryService& inventoryService,
    service::OrderService& orderService,
    service::PrepaymentService& prepaymentService,
    service::MessageService& messageService,
    service::DistanceService& distanceService,
    service::ErrorService& errorService,
    boost::asio::io_context& ioContext,
    const std::string& myVmId,
    int myVmX,
    int myVmY
) : userInterface_(ui),
    inventoryService_(inventoryService),
    orderService_(orderService),
    prepaymentService_(prepaymentService),
    messageService_(messageService),
    distanceService_(distanceService),
    errorService_(errorService),
    ioContext_(ioContext),
    myVendingMachineId_(myVmId),
    myVendingMachineX_(myVmX),
    myVendingMachineY_(myVmY),
    currentState_(ControllerState::INITIALIZING),
    isCurrentOrderPrepayment_(false),
    total_other_vms_(7) { // 지금 프로젝트가 8팀이기 떄문에 현재 자판기 제외한 7개 응답을 기준으로 정함
    // MessageService 핸들러 등록은 initializeSystemAndRegisterMessageHandlers에서 수행
}

/**
 * @brief 자판기 시스템의 메인 실행 루프.
 * 초기화 후, 현재 상태에 따라 작업을 수행하고 사용자 입력을 처리하며,
 * 네트워크 이벤트를 주기적으로 폴링합니다.
 */
void UserProcessController::run() {
    if (currentState_ == ControllerState::INITIALIZING) {
        initializeSystemAndRegisterMessageHandlers();
        userInterface_.displayMessage("자판기 시스템을 시작합니다. 현재 자판기 ID: " + myVendingMachineId_);
        changeState(ControllerState::SYSTEM_READY);
    }

    while (currentState_ != ControllerState::SYSTEM_HALTED_REQUEST) {
        processCurrentState(); // 현재 상태에 따른 작업 수행

        if (currentState_ != ControllerState::SYSTEM_HALTED_REQUEST) {
            // 네트워크 이벤트 처리 (io_context 큐에 있는 핸들러 실행 시도)
            // 현재는 각 상태 처리 후 짧게 폴링.
            for (int i = 0; i < 5; ++i) { // 짧은 시간 동안 여러 이벤트 처리 시도
                if (ioContext_.stopped()) {
                    ioContext_.restart(); // io_context가 중지된 경우 재시작
                }
                ioContext_.poll_one(); // 논블로킹으로 하나의 핸들러 실행
                if (currentState_ == ControllerState::SYSTEM_HALTED_REQUEST) break;
            }
        }
    }
    userInterface_.displayMessage("자판기 시스템을 종료합니다.");
}

/**
 * @brief 컨트롤러의 현재 상태를 변경합니다.
 * @param newState 새로운 ControllerState.
 */
void UserProcessController::changeState(ControllerState newState) {
    currentState_ = newState;
}

/**
 * @brief 현재 진행 중인 거래와 관련된 임시 상태 정보들을 초기화합니다.
 * 새로운 거래 시작 또는 거래 완료/취소 시 호출됩니다.
 */
void UserProcessController::resetCurrentTransactionState() {
    currentActiveOrder_.reset();
    isCurrentOrderPrepayment_ = false;
    pendingDrinkSelection_.reset();
    availableOtherVmsForDrink_.clear();
    selectedTargetVmForPrepayment_.reset();
    timeout_start_time_.reset(); // 타임아웃 관련 상태도 초기화
}

/**
 * @brief 음료 코드를 사용하여 음료의 상세 정보(이름, 가격 등)를 조회합니다.
 * @param drinkCode 조회할 음료의 코드.
 * @return 해당 음료의 domain::Drink 객체.
 * 찾지 못하거나 오류 발생 시 빈 Drink 객체를 반환하고 오류 처리 상태로 전환합니다.
 */
domain::Drink UserProcessController::getDrinkDetails(const std::string& drinkCode) {
    try {
        auto allDrinks = inventoryService_.getAllDrinkTypes(); // 전체 음료 목록 조회
        for (const auto& drink : allDrinks) {
            if (drink.getDrinkCode() == drinkCode) {
                return drink; // 해당 음료 반환
            }
        }
        // PFR R1.1 (전체 20종 음료 메뉴 제공)에 따라 목록에 있어야 함.
        // 목록에 없는 코드가 입력된 경우 (예: 사용자가 직접 잘못 입력)
        auto errorInfo = errorService_.processOccurredError(ErrorType::DRINK_NOT_FOUND, "음료 코드(" + drinkCode + ")가 메뉴에 없습니다.");
        state_handlingError(errorInfo);
        return domain::Drink(); // 빈 객체 반환
    } catch (const std::exception& e) { // DrinkRepository 접근 등 예외
        auto errorInfo = errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "음료 정보 조회 중 시스템 오류: " + std::string(e.what()));
        state_handlingError(errorInfo);
        return domain::Drink();
    }
}

/**
 * @brief 시스템 시작 시 필요한 초기화 작업 및 MessageService에 콜백 핸들러를 등록합니다.
 */
void UserProcessController::initializeSystemAndRegisterMessageHandlers() {
    // MessageService를 통해 각 메시지 타입에 대한 핸들러(콜백) 등록
    // 콜백은 io_context를 통해 실행되도록 post하여 메인 스레드(또는 io_context 실행 스레드)에서 안전하게 처리
    messageService_.registerMessageHandler(network::Message::Type::REQ_STOCK,
        [this](const network::Message& msg){ boost::asio::post(ioContext_, [this, msg](){ this->onReqStockReceived(msg); }); });
    messageService_.registerMessageHandler(network::Message::Type::RESP_STOCK,
        [this](const network::Message& msg){ boost::asio::post(ioContext_, [this, msg](){ this->onRespStockReceived(msg); }); });
    messageService_.registerMessageHandler(network::Message::Type::REQ_PREPAY,
        [this](const network::Message& msg){ boost::asio::post(ioContext_, [this, msg](){ this->onReqPrepayReceived(msg); }); });
    messageService_.registerMessageHandler(network::Message::Type::RESP_PREPAY,
        [this](const network::Message& msg){ boost::asio::post(ioContext_, [this, msg](){ this->onRespPrepayReceived(msg); }); });

    messageService_.startReceivingMessages(); // 네트워크 메시지 수신 시작
}

/**
 * @brief 현재 컨트롤러 상태에 따라 적절한 상태 처리 함수를 호출합니다.
 * 이 함수는 메인 루프(`run()`)에서 반복적으로 호출되어 상태 머신을 구동합니다.
 */
void UserProcessController::processCurrentState() {
    switch (currentState_) {
        case ControllerState::SYSTEM_READY:
            changeState(ControllerState::DISPLAYING_MAIN_MENU);
            break;
        case ControllerState::DISPLAYING_MAIN_MENU:       // UC1
            state_displayingMainMenu();
            break;
        case ControllerState::AWAITING_DRINK_SELECTION:   // UC2, UC3
            state_awaitingDrinkSelection();
            break;
        case ControllerState::AWAITING_PAYMENT_CONFIRMATION: // UC4, UC11
            state_awaitingPaymentConfirmation();
            break;
        case ControllerState::PROCESSING_PAYMENT:         // UC4, UC5, UC6
            state_processingPayment();
            break;
        case ControllerState::DISPENSING_DRINK:           // UC7, UC14
            state_dispensingDrink();
            break;
        case ControllerState::BROADCASTING_STOCK_REQUEST: // UC8
            state_broadcastingStockRequest();
            break;
        case ControllerState::AWAITING_STOCK_RESPONSES:   // UC9 (응답 대기 및 타임아웃)
            if (timeout_start_time_) { // 타임아웃 타이머가 활성화된 경우
                if (std::chrono::steady_clock::now() - *timeout_start_time_ >= current_timeout_duration_) {
                    timeout_start_time_.reset(); // 타임아웃 발생, 타이머 비활성화
                    if (availableOtherVmsForDrink_.empty()) { // UC9 E2
                        userInterface_.displayNoOtherVendingMachineFound(pendingDrinkSelection_ ? pendingDrinkSelection_->getName() : "선택 음료");
                        state_handlingError(errorService_.processOccurredError(ErrorType::RESPONSE_TIMEOUT_FROM_OTHER_VM, "주변 자판기 재고 조회"));
                    } else { // 타임아웃되었으나, 일부 응답이라도 수신된 경우
                        userInterface_.displayMessage("응답 시간이 초과되었으나, 수신된 정보로 안내합니다.");
                        changeState(ControllerState::DISPLAYING_OTHER_VM_OPTIONS); // UC10으로
                    }
                }
                // 타임아웃 전이면 run() 루프의 io_context_.poll_one()을 통해 콜백 처리 대기
            }
            break;
        case ControllerState::DISPLAYING_OTHER_VM_OPTIONS: // UC10, UC11
            state_displayingOtherVmOptions();
            break;
        case ControllerState::ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION: // UC16 (응답 대기 및 타임아웃)
            if (timeout_start_time_) {
                if (std::chrono::steady_clock::now() - *timeout_start_time_ >= current_timeout_duration_) {
                    timeout_start_time_.reset(); // 타임아웃 발생
                    std::string targetVmId_str = selectedTargetVmForPrepayment_ ? selectedTargetVmForPrepayment_->getId() : "대상 자판기";
                    state_handlingError(errorService_.processOccurredError(ErrorType::RESPONSE_TIMEOUT_FROM_OTHER_VM, targetVmId_str + "로부터 선결제 응답 없음"));
                }
            }
            break;
        case ControllerState::DISPLAYING_AUTH_CODE_INFO:  // UC12
            state_displayingAuthCodeInfo();
            break;
        case ControllerState::AWAITING_AUTH_CODE_INPUT_PROMPT: // UC13, UC14
            state_awaitingAuthCodeInputPrompt();
            break;
        case ControllerState::TRANSACTION_COMPLETED_RETURN_TO_MENU:
            state_transactionCompletedReturnToMenu();
            break;
        case ControllerState::HANDLING_ERROR:
            // state_handlingError 함수 내부에서 다음 상태(주로 DISPLAYING_MAIN_MENU)로 전이
            break;
        default: // 정의되지 않은 상태
            state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "알 수 없는 컨트롤러 상태 값: " + std::to_string(static_cast<int>(currentState_))));
            break;
    }
}

// --- 유스케이스별 상태 처리 함수들 ---

/**
 * @brief UC1: 메인 메뉴를 표시하고 사용자 입력을 받아 다음 상태로 전환합니다.
 */
void UserProcessController::state_displayingMainMenu() {
    resetCurrentTransactionState(); // 새 거래 시작 전 이전 상태 초기화
    // UC1 Typical Course 1 & 2: 음료 목록 조회 및 표시 (PFR R1.1)
    std::vector<domain::Drink> allDrinks = inventoryService_.getAllDrinkTypes();
    userInterface_.displayMessage("\n=========== Vending Machine Menu ===========");
    userInterface_.displayDrinkList(allDrinks, {}); // 현재 자판기 재고는 이 단계에서 미표시

    // UC1 Typical Course 3: 사용자에게 선택 화면 제공 및 입력 대기
    std::vector<std::string> menuOptions = {
        "1. 음료 선택 (구매/다른 자판기 조회)",
        "2. 인증 코드로 음료 받기",
        "3. 시스템 종료"
    };
    userInterface_.displayMainMenu(menuOptions);
    int choice = userInterface_.getUserChoice(menuOptions.size());

    switch (choice) {
        case 1: changeState(ControllerState::AWAITING_DRINK_SELECTION); break; // UC2로
        case 2: changeState(ControllerState::AWAITING_AUTH_CODE_INPUT_PROMPT); break; // UC13으로
        case 3: changeState(ControllerState::SYSTEM_HALTED_REQUEST); break;
        default: 
            state_handlingError(errorService_.processOccurredError(ErrorType::INVALID_MENU_CHOICE, "메인 메뉴 선택"));
            break;
    }
}

/**
 * @brief UC2, UC3: 사용자로부터 음료 선택을 받고, 현재 자판기의 재고를 확인하여 다음 상태로 전환합니다.
 */
void UserProcessController::state_awaitingDrinkSelection() {
    timeout_start_time_.reset(); // 타임아웃 불필요

    // (A) UC2.1: 사용자가 음료 선택
    std::string drinkCode = userInterface_.selectDrink(inventoryService_.getAllDrinkTypes(), {});
    if (drinkCode.empty()) { // 사용자가 'c' 입력 등으로 선택 취소
        userInterface_.displayMessage("음료 선택이 취소되었습니다.");
        changeState(ControllerState::DISPLAYING_MAIN_MENU);
        return;
    }

    pendingDrinkSelection_ = getDrinkDetails(drinkCode);
    if (!pendingDrinkSelection_ || pendingDrinkSelection_->getDrinkCode().empty()) {
        return; // getDrinkDetails 내부에서 오류 처리 및 상태 변경됨
    }

    // (S) UC2.2 & UC3.1: 선택 음료 판매 가능 여부 및 재고 확인 (PFR R1.3)
    auto availability = inventoryService_.checkDrinkAvailabilityAndPrice(drinkCode);

    if (availability.isAvailable) { // (S) UC3.2: 재고 있음
        userInterface_.displayMessage(
            pendingDrinkSelection_->getName() + " 선택됨. 가격: " + std::to_string(availability.price) +
            "원. (재고: " + std::to_string(availability.currentStock) + "개)"
        );
        currentActiveOrder_ = orderService_.createOrder(myVendingMachineId_, *pendingDrinkSelection_);
        isCurrentOrderPrepayment_ = false; // 일반 구매
        changeState(ControllerState::AWAITING_PAYMENT_CONFIRMATION); // UC4로
    } else { // (S) UC3.3: 재고 없음 또는 판매 안 함
        userInterface_.displayOutOfStockMessage(pendingDrinkSelection_->getName());
        changeState(ControllerState::BROADCASTING_STOCK_REQUEST); // UC8로
    }
    // UC2 E1 (사용자 응답 시간 초과)는 UserInterface::selectDrink가 블로킹 방식이라 컨트롤러 레벨에서 별도 처리 안 함.
    // UC3 E1 (재고 정보 로드 실패)은 InventoryService 내부에서 ErrorService로 처리.
}

/**
 * @brief UC4, UC11: 사용자에게 결제(또는 선결제) 진행 여부를 확인하고 다음 상태로 전환합니다.
 */
void UserProcessController::state_awaitingPaymentConfirmation() {
    timeout_start_time_.reset();
    if (!currentActiveOrder_ || !pendingDrinkSelection_) {
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "결제 확인 단계: 필수 정보 누락"));
        return;
    }

    int price = pendingDrinkSelection_->getPrice();
    std::string action_type = isCurrentOrderPrepayment_ ? "선결제" : "구매";
    std::string target_info = "";
    if(isCurrentOrderPrepayment_ && selectedTargetVmForPrepayment_){
        target_info = " (대상 자판기: " + selectedTargetVmForPrepayment_->getId() + ")";
    }
    userInterface_.displayMessage(pendingDrinkSelection_->getName() + " " + action_type + target_info + ". 가격: " + std::to_string(price) + "원.");

    // (S) UC4.1 / UC11.1: 결제 요청 안내
    userInterface_.displayPaymentPrompt(price);
    // (A) UC4.2 / UC11.2: 사용자 결제 시도 여부 확인
    if (userInterface_.confirmPayment()) {
        changeState(ControllerState::PROCESSING_PAYMENT);
    } else {
        userInterface_.displayMessage(action_type + "가 취소되었습니다.");
        changeState(ControllerState::DISPLAYING_MAIN_MENU);
    }
}

/**
 * @brief UC4, UC5, UC6: 실제 결제를 시도하고 그 결과에 따라 다음 상태로 전환합니다.
 */
void UserProcessController::state_processingPayment() {
    timeout_start_time_.reset();
    if (!currentActiveOrder_ || !pendingDrinkSelection_) {
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "결제 처리 단계: 필수 정보 누락"));
        return;
    }

    userInterface_.displayPaymentProcessing(); // "결제 진행 중..."
    network::PaymentCallbackReceiver paymentSim; // 결제 시뮬레이터
    bool paymentSuccess = false;
    // (S) UC4.3: CardSystem 응답 대기 (시뮬레이션)
    paymentSim.simulatePrepayment([&paymentSuccess](bool success) {
        paymentSuccess = success;
    }, 3); // 3초 지연 후 콜백 실행

    if (paymentSuccess) { // (S) UC5.1: 승인 응답
        userInterface_.displayPaymentResult(true, "결제가 성공적으로 완료되었습니다!");
        orderService_.processOrderApproval(*currentActiveOrder_, isCurrentOrderPrepayment_); // (S) UC5.2

        if (isCurrentOrderPrepayment_) { // (S) UC5.2: 선결제 시
            if (!selectedTargetVmForPrepayment_) { // 방어 코드
                state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "선결제 대상 자판기 정보가 없습니다."));
                return;
            }
            changeState(ControllerState::ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION); // UC16으로
        } else { // (S) UC5.3: 일반 결제 시
            changeState(ControllerState::DISPENSING_DRINK); // UC7로
        }
    } else { // (S) UC6.1: DECLINED 응답 (또는 UC4 E1/E2 - 시뮬레이션에서는 단순 실패로 처리)
        userInterface_.displayPaymentResult(false, "결제에 실패했습니다. 카드 정보를 확인하거나 다른 카드를 사용해주세요.");
        orderService_.processOrderDeclination(*currentActiveOrder_); // (S) UC6.2
        changeState(ControllerState::DISPLAYING_MAIN_MENU); // (S) UC6.4
    }
}

/**
 * @brief UC7, UC14: 음료를 배출하고 거래 완료 상태로 전환합니다.
 */
void UserProcessController::state_dispensingDrink() {
    timeout_start_time_.reset();
    if (!pendingDrinkSelection_ || !currentActiveOrder_) {
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "음료 배출 단계: 필수 정보 누락"));
        return;
    }

    userInterface_.displayDispensingDrink(pendingDrinkSelection_->getName()); // (S) UC7.1
    std::this_thread::sleep_for(std::chrono::seconds(2)); // 배출 시간 시뮬레이션
    userInterface_.displayDrinkDispensed(pendingDrinkSelection_->getName()); // (S) UC7.3

    if (isCurrentOrderPrepayment_) { // 선결제 음료 수령 시 (UC14의 일부)
        prepaymentService_.changeAuthCodeStatusToUsed(currentActiveOrder_->getCertCode()); // (S) UC14.2: AuthCode 만료
    }
    // 일반 구매 시 재고 차감(UC7.2)은 OrderService::processOrderApproval에서 이미 처리됨.
    changeState(ControllerState::TRANSACTION_COMPLETED_RETURN_TO_MENU);
}

/**
 * @brief UC8: 선택한 음료가 현재 자판기에 없을 경우, 주변 자판기에 재고 조회 메시지를 브로드캐스트합니다.
 */
void UserProcessController::state_broadcastingStockRequest() {
    if (!pendingDrinkSelection_) {
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "재고 조회 요청: 음료 정보 없음"));
        return;
    }
    userInterface_.displayMessage("선택하신 음료 [" + pendingDrinkSelection_->getName() + "]의 재고를 주변 자판기에 문의합니다...");
    messageService_.sendStockRequestBroadcast(pendingDrinkSelection_->getDrinkCode()); // (S) UC8.1
    availableOtherVmsForDrink_.clear(); // 이전 결과 초기화

    timeout_start_time_ = std::chrono::steady_clock::now(); // (S) UC8.2 -> UC9 E2 (타임아웃 시작)
    current_timeout_duration_ = std::chrono::seconds(5); // PFR 및 유스케이스 문서 기준
    changeState(ControllerState::AWAITING_STOCK_RESPONSES);
}

/**
 * @brief UC10, UC11: 다른 자판기로부터 수신된 재고 정보를 바탕으로 가장 가까운 자판기를 안내하고,
 * 사용자에게 해당 자판기에서의 선결제 여부를 확인합니다.
 */
void UserProcessController::state_displayingOtherVmOptions() {
    timeout_start_time_.reset();
    if (!pendingDrinkSelection_) {
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "다른 자판기 옵션 표시: 음료 정보 없음"));
        return;
    }
    if (availableOtherVmsForDrink_.empty()) { // UC9 결과, 재고 있는 다른 자판기 없음
        userInterface_.displayNoOtherVendingMachineFound(pendingDrinkSelection_->getName());
        changeState(ControllerState::DISPLAYING_MAIN_MENU);
        return;
    }

    try { // UC10 (S)-1: 가장 가까운 자판기 계산 및 안내 
        std::optional<domain::VendingMachine> nearestVm = distanceService_.findNearestAvailableVendingMachine(
            myVendingMachineX_, myVendingMachineY_, availableOtherVmsForDrink_
        );
        if (nearestVm) {
            selectedTargetVmForPrepayment_ = nearestVm;
            userInterface_.displayNearestVendingMachine(*selectedTargetVmForPrepayment_, pendingDrinkSelection_->getName());

            // UC11 (S)-1: 사용자에게 선결제 여부 문의 
            if (userInterface_.confirmPrepayment(pendingDrinkSelection_->getName())) { // UC11 (A)-2
                currentActiveOrder_ = orderService_.createOrder(myVendingMachineId_, *pendingDrinkSelection_);
                isCurrentOrderPrepayment_ = true;
                changeState(ControllerState::AWAITING_PAYMENT_CONFIRMATION); // UC11 (S)-3 -> UC4
            } else { // 사용자가 선결제 거부
                userInterface_.displayMessage("선결제가 취소되었습니다.");
                changeState(ControllerState::DISPLAYING_MAIN_MENU);
            }
        } else { // 구매 가능한 가까운 자판기 없음
            userInterface_.displayNoOtherVendingMachineFound(pendingDrinkSelection_->getName());
            state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "구매 가능한 가까운 자판기를 선택할 수 없습니다."));
        }
    } catch (const std::exception& e) { // DistanceService 내부 오류 등
        userInterface_.displayNoOtherVendingMachineFound(pendingDrinkSelection_->getName());
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "가까운 자판기 처리 중 예외: " + std::string(e.what())));
    }
}

/**
 * @brief UC16: 선결제가 승인된 후, 선택된 다른 자판기에 재고 확보 및 인증 코드 등록을 요청합니다.
 */
void UserProcessController::state_issuingAuthCodeAndRequestingReservation() {
    if (!currentActiveOrder_ || currentActiveOrder_->getCertCode().empty() || !pendingDrinkSelection_ || !selectedTargetVmForPrepayment_ || !isCurrentOrderPrepayment_) {
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "선결제 재고 확보 요청: 정보 부족"));
        return;
    }
    // UC16 (S)-1: 수령 자판기에 메시지 전송 
    userInterface_.displayMessage(selectedTargetVmForPrepayment_->getId() + "에 " + pendingDrinkSelection_->getName() +
                                " 재고 확보 요청 (인증코드: " + currentActiveOrder_->getCertCode() + ")");
    messageService_.sendPrepaymentReservationRequest(
        selectedTargetVmForPrepayment_->getId(),
        currentActiveOrder_->getDrinkCode(),
        currentActiveOrder_->getCertCode()
    );
    timeout_start_time_ = std::chrono::steady_clock::now();
    current_timeout_duration_ = std::chrono::seconds(30); // 선결제 재고 확보 응답 대기 시간 (임의 설정)
    // 상태는 ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION 유지, onRespPrepayReceived 또는 타임아웃으로 다음 상태 결정
}

/**
 * @brief UC12: 다른 자판기에서 재고 확보 성공 후, 사용자에게 인증 코드와 수령 정보를 안내합니다.
 */
void UserProcessController::state_displayingAuthCodeInfo() {
    timeout_start_time_.reset(); // 이전 타임아웃 해제

    if (!currentActiveOrder_ || currentActiveOrder_->getCertCode().empty() || !selectedTargetVmForPrepayment_ || !pendingDrinkSelection_) {
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드 안내: 정보 부족"));
        return;
    }

    // UC12 (S)-2, (S)-3: AuthCode 및 안내 메시지 표시
    // PFR R4.2: 선결제 완료 시 인증 코드를 생성하고 출력한다.
    // PFR: "선결제를 마친 후 사용자에게 인증코드와 선결제를 마친 자판기의 위치 정보(좌표) 등 필요한 정보를 안내해야 한다."
    
    // currentActiveOrder_에서 인증 코드 문자열을 가져와 전달합니다.
    userInterface_.displayAuthCode(
        currentActiveOrder_->getCertCode(), // *currentActiveOrder_ 대신 currentActiveOrder_->getCertCode() 사용
        *selectedTargetVmForPrepayment_,    // 위치 정보 포함된 VendingMachine 객체
        pendingDrinkSelection_->getName()
    );

    changeState(ControllerState::TRANSACTION_COMPLETED_RETURN_TO_MENU);
}

/**
 * @brief UC13, UC14: 사용자로부터 인증 코드를 입력받아 유효성을 검증하고, 성공 시 음료 배출 상태로 전환합니다.
 */
void UserProcessController::state_awaitingAuthCodeInputPrompt() {
    timeout_start_time_.reset();
    std::string authCode = userInterface_.getAuthCodeInput(); // UC13 (A)-1, (A)-2

    if (authCode.empty()) { // 입력 취소
        userInterface_.displayMessage("인증 코드 입력이 취소되었습니다.");
        changeState(ControllerState::DISPLAYING_MAIN_MENU);
        return;
    }
    if (!prepaymentService_.isValidAuthCodeFormat(authCode)) { // UC13 (S)-3
        state_handlingError(errorService_.processOccurredError(ErrorType::AUTH_CODE_INVALID_FORMAT, authCode)); // UC13 E1
        return;
    }

    // UC13 (S)-4 -> UC14
    domain::PrePaymentCode prepayDetails = prepaymentService_.getPrepaymentDetailsIfActive(authCode); // UC14 (S)-1
    if (prepayDetails.getCode().empty()) { // AUTH_CODE_NOT_FOUND 또는 AUTH_CODE_ALREADY_USED (UC14 E1)
        // ErrorService에서 이미 오류 알림, 여기서는 메인 메뉴로
        changeState(ControllerState::DISPLAYING_MAIN_MENU);
        return;
    }

    std::shared_ptr<domain::Order> heldOrderPtr = prepayDetails.getHeldOrder();
    if (heldOrderPtr) {
        currentActiveOrder_ = *heldOrderPtr;
    } else {
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드에 연결된 주문 객체 없음"));
        return;
    }
     if(!currentActiveOrder_){ 
         state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드에 연결된 주문 정보 없음"));
        return;
    }
    pendingDrinkSelection_ = getDrinkDetails(currentActiveOrder_->getDrinkCode());
    if (!pendingDrinkSelection_ || pendingDrinkSelection_->getDrinkCode().empty()) {
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드의 음료 정보 조회 실패"));
        return;
    }
    
    isCurrentOrderPrepayment_ = true; // 인증 코드로 받는 것은 선결제된 음료
    userInterface_.displayMessage("인증 코드 (" + authCode + ") 확인 완료: " + pendingDrinkSelection_->getName());
    changeState(ControllerState::DISPENSING_DRINK); // UC14 (S)-3 -> UC7
}

/**
 * @brief 현재 거래를 성공적으로 완료하고 관련 상태를 초기화한 후 메인 메뉴로 돌아갑니다.
 */
void UserProcessController::state_transactionCompletedReturnToMenu() {
    timeout_start_time_.reset();
    userInterface_.displayMessage("거래가 완료되었습니다. 감사합니다.");
    resetCurrentTransactionState();
    changeState(ControllerState::DISPLAYING_MAIN_MENU);
}

/**
 * @brief 발생한 오류에 대한 정보를 바탕으로 사용자에게 메시지를 표시하고,
 * ErrorResolutionLevel에 따라 다음 상태로 전환합니다.
 * @param errorInfo ErrorService로부터 받은 오류 정보 객체.
 */
void UserProcessController::state_handlingError(const service::ErrorInfo& errorInfo) {
    timeout_start_time_.reset(); // 진행 중이던 타임아웃이 있다면 해제
    userInterface_.displayError(errorInfo.userFriendlyMessage);

    switch (errorInfo.resolutionLevel) {
        case ErrorResolutionLevel::RETRY_INPUT: 
            userInterface_.displayMessage("입력을 다시 시도해주세요. (메인 메뉴로 돌아갑니다)");
            changeState(ControllerState::DISPLAYING_MAIN_MENU);
            break;
        case ErrorResolutionLevel::SYSTEM_FATAL_ERROR: // 시스템 종료 필요
            userInterface_.displayMessage("치명적인 시스템 오류가 발생하여 시스템을 종료합니다.");
            changeState(ControllerState::SYSTEM_HALTED_REQUEST);
            break;
        case ErrorResolutionLevel::RETURN_TO_MAIN_MENU:
            userInterface_.displayMessage("메인 메뉴로 돌아갑니다.");
            changeState(ControllerState::DISPLAYING_MAIN_MENU);
            break;
        default:
            userInterface_.displayMessage("초기 화면으로 돌아갑니다.");
            changeState(ControllerState::DISPLAYING_MAIN_MENU);
            break;
    }
}

// --- 콜백 핸들러들 ---

/**
 * @brief UC17: 다른 자판기로부터 재고 조회 요청(REQ_STOCK)을 수신했을 때 호출됩니다.
 * 요청받은 음료의 현재 재고를 확인하여 응답(RESP_STOCK)합니다.
 * @param msg 수신된 REQ_STOCK 메시지.
 */
void UserProcessController::onReqStockReceived(const network::Message& msg) {
    std::cout << "[" << myVendingMachineId_ << "] Received REQ_STOCK from " << msg.src_id << " for item " << msg.msg_content.at("item_code") << std::endl;

    // (S) UC17.1: "req_stock"를 수신한다.
    if (msg.msg_content.count("item_code")) {
        std::string requestedDrinkCode = msg.msg_content.at("item_code");
        // (S) UC17.2: 재고 값을 조회한다.
        auto availabilityInfo = inventoryService_.checkDrinkAvailabilityAndPrice(requestedDrinkCode);
        // (S) UC17.3: "resp_stock"타입 메세지를 전송한다. 
        messageService_.sendStockResponse(msg.src_id, requestedDrinkCode, availabilityInfo.currentStock);
    } else { // UC17 E1: 메시지를 읽을 수 없으면 시스템은 에러를 호출하고 무응답한다.
        errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "REQ_STOCK (item_code 누락 from " + msg.src_id + ")");
    }
        std::cout << "[" << myVendingMachineId_ << "] Sending RESP_STOCK to " << msg.src_id
          << " for item with stock stockCount"<< std::endl;
        messageService_.sendStockResponse(msg.src_id, msg.msg_content.at("item_code"), 0); // 재고가 없을 경우 0으로 응답
}

/**
 * @brief UC9: 현재 자판기가 보낸 재고 조회 브로드캐스트에 대한 응답(RESP_STOCK)을 다른 자판기로부터 수신했을 때 호출됩니다.
 * 유효한 응답들을 수집하고, 모든 응답을 받거나 타임아웃 시 다음 단계로 진행합니다.
 * @param msg 수신된 RESP_STOCK 메시지.
 */
void UserProcessController::onRespStockReceived(const network::Message& msg) {
    if (currentState_ != ControllerState::AWAITING_STOCK_RESPONSES) {
        return; // 현재 응답 대기 상태가 아니면 무시
    }

    try {
        std::string drinkCode = msg.msg_content.at("item_code");
        int stockQty = std::stoi(msg.msg_content.at("item_num")); // PFR 표2
        int x = std::stoi(msg.msg_content.at("coor_x"));
        int y = std::stoi(msg.msg_content.at("coor_y"));
        std::string vmId = msg.src_id;

        // (A) UC9.1: 재고 있는 자판기가 응답
        if (pendingDrinkSelection_ && pendingDrinkSelection_->getDrinkCode() == drinkCode && stockQty > 0) {
            bool alreadyReceived = false;
            for(const auto& ovm : availableOtherVmsForDrink_) if(ovm.id == vmId) alreadyReceived = true;
            if(alreadyReceived) return; // 중복 응답 무시

            availableOtherVmsForDrink_.push_back({vmId, x, y, true}); // service::OtherVendingMachineInfo 사용
            userInterface_.displayMessage(vmId + "에서 " + pendingDrinkSelection_->getName() + " 재고(" + std::to_string(stockQty) + "개) 확인.");

            if (availableOtherVmsForDrink_.size() >= total_other_vms_) { // 모든 다른 자판기로부터 응답 가정
                if(timeout_start_time_){ // 타임아웃 전이라면
                    timeout_start_time_.reset(); // 타임아웃 중지
                    userInterface_.displayMessage("모든 주변 자판기로부터 응답을 확인했습니다.");
                    changeState(ControllerState::DISPLAYING_OTHER_VM_OPTIONS); // (S) UC9.2 -> UC10
                }
            }
        }
    } catch (const std::exception& e) { // UC9 E1: 메시지 손상 시 무시
        errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "RESP_STOCK (from " + msg.src_id + ", 무시됨): " + e.what());
    }
}

/**
 * @brief UC15: 다른 자판기로부터 선결제 요청(REQ_PREPAY)을 수신했을 때 호출됩니다.
 * 요청받은 음료의 재고를 확인 및 확보(차감)하고, 인증코드를 등록한 후 응답(RESP_PREPAY)합니다.
 * @param msg 수신된 REQ_PREPAY 메시지.
 */
void UserProcessController::onReqPrepayReceived(const network::Message& msg) {
    try {
        std::cout << "[" << myVendingMachineId_ << "] Received REQ_PREPAY from " << msg.src_id
              << " for item " << msg.msg_content.at("item_code")
              << " with cert_code " << msg.msg_content.at("cert_code") << std::endl;
        // (S) UC15.1: REQ_PREPAY 메시지 수신
        std::string drinkCode = msg.msg_content.at("item_code");
        std::string certCode = msg.msg_content.at("cert_code");
        std::string requestingVmId = msg.src_id;
        int requestedItemNum = 1; // 기본값
        if (msg.msg_content.count("item_num")) {
            try {
                requestedItemNum = std::stoi(msg.msg_content.at("item_num"));
                if (requestedItemNum <= 0 || requestedItemNum > 99) { // PFR 재고 범위
                    errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "REQ_PREPAY (잘못된 item_num " + std::to_string(requestedItemNum) + " from " + msg.src_id + ")");
                    messageService_.sendPrepaymentReservationResponse(requestingVmId, drinkCode, 0, false); // 실패 응답
                    return;
                }
            } catch (const std::exception&) {
                 errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "REQ_PREPAY (item_num 파싱 오류 from " + msg.src_id + ")");
                 messageService_.sendPrepaymentReservationResponse(requestingVmId, drinkCode, 0, false); // 실패 응답
                 return;
            }
        }

        bool reservationSuccess = false;
        auto availabilityInfo = inventoryService_.checkDrinkAvailabilityAndPrice(drinkCode);

        // (S) UC15.2: 재고 확인 및 확보
        if (availabilityInfo.isAvailable && availabilityInfo.currentStock >= requestedItemNum) {
            inventoryService_.decreaseStockByAmount(drinkCode, requestedItemNum); // 요청 수량만큼 재고 차감
            prepaymentService_.recordIncomingPrepayment(certCode, drinkCode, requestingVmId); // 인증코드 및 주문 정보 기록
            reservationSuccess = true;
            userInterface_.displayMessage(requestingVmId + "로부터 " + drinkCode + " " + std::to_string(requestedItemNum) + "개 선결제 처리 완료 (인증코드: " + certCode + ")");
        } else { // (A1) UC15: 재고 부족
            userInterface_.displayMessage(requestingVmId + "로부터 " + drinkCode + " " + std::to_string(requestedItemNum) + "개 선결제 처리 실패 (재고 부족)");
        }
        // (S) UC15.3: 응답 메시지 송신 (PFR 표4)
        std::cout << "[" << myVendingMachineId_ << "] Sending RESP_PREPAY to " << requestingVmId
              << " with availability: " << (reservationSuccess ? "T" : "F")
              << ", item_num: " << (reservationSuccess ? requestedItemNum : 0) << std::endl;
        messageService_.sendPrepaymentReservationResponse(requestingVmId, drinkCode, (reservationSuccess ? requestedItemNum : 0), reservationSuccess);
    } catch (const std::exception& e) { // 메시지 형식 오류 등
        auto errorInfo = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "REQ_PREPAY 처리 오류 (from " + msg.src_id + "): " + e.what());
    }
}

/**
 * @brief UC16: 현재 자판기가 보낸 선결제 재고 확보 요청에 대한 응답(RESP_PREPAY)을 다른 자판기로부터 수신했을 때 호출됩니다.
 * 응답 결과에 따라 다음 단계(인증코드 안내 또는 오류 처리)로 진행합니다.
 * @param msg 수신된 RESP_PREPAY 메시지.
 */
void UserProcessController::onRespPrepayReceived(const network::Message& msg) {
     std::cout << "[" << myVendingMachineId_ << "] Received RESP_PREPAY from " << msg.src_id
              << " with availability: " << msg.msg_content.at("availability")
              << ". Current state: " << static_cast<int>(currentState_) << std::endl;

    if (currentState_ != ControllerState::ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION) {
        return; // 현재 해당 응답을 기다리는 상태가 아니면 무시
    }
    timeout_start_time_.reset(); // 응답 수신, 타임아웃 중지

    try {
        std::string receivedDrinkCode = msg.msg_content.at("item_code");
        std::string availability = msg.msg_content.at("availability"); // "T" 또는 "F" (PFR 표4)
        // PFR 표4에 item_num도 있지만, 이 응답에서는 availability("T"/"F")가 주된 판단 기준.

        // 현재 진행 중인 선결제 요청과 응답 내용이 일치하는지 확인
        if (pendingDrinkSelection_ && pendingDrinkSelection_->getDrinkCode() == receivedDrinkCode &&
            currentActiveOrder_ && !currentActiveOrder_->getCertCode().empty() &&
            selectedTargetVmForPrepayment_ && selectedTargetVmForPrepayment_->getId() == msg.src_id) {

            if (availability == "T") { // (S) UC16.2: 재고 확보 성공 응답
                userInterface_.displayMessage(msg.src_id + " 자판기에서 재고 확보에 성공했습니다.");
                changeState(ControllerState::DISPLAYING_AUTH_CODE_INFO); // UC12 (인증코드 안내)로 이동
            } else { // (E1) UC16: 재고 확보 실패 응답 (availability == "F")
                auto errorInfo = errorService_.processOccurredError(ErrorType::STOCK_RESERVATION_FAILED_AT_OTHER_VM, msg.src_id);
                state_handlingError(errorInfo);
            }
        } else { // 응답 내용이 현재 진행 중인 요청과 불일치
            auto errorInfo = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "RESP_PREPAY (내용 불일치 from " + msg.src_id + ")");
            state_handlingError(errorInfo);
        }
    } catch (const std::exception& e) { // 메시지 형식 오류 등
        auto errorInfo = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "RESP_PREPAY 처리 오류 (from " + msg.src_id + "): " + e.what());
        state_handlingError(errorInfo);
    }
}

} // namespace service
