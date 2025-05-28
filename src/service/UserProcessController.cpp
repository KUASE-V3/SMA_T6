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
#include "domain/prepaymentCode.h" // domain::PrePaymentCode 사용
#include "network/message.hpp"
#include "network/PaymentCallbackReceiver.hpp" // 직접 사용하므로 include

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <thread> // std::this_thread::sleep_for

// UserProcessController.hpp에 DistanceService.hpp가 이미 포함되어 있다고 가정
// (OtherVendingMachineInfo 사용을 위해)

namespace service {

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
    isCurrentOrderPrepayment_(false) {
    // MessageService 핸들러 등록은 initializeSystemAndRegisterMessageHandlers에서 수행
}

void UserProcessController::run() {
    if (currentState_ == ControllerState::INITIALIZING) {
        initializeSystemAndRegisterMessageHandlers();
        userInterface_.displayMessage("자판기 시스템을 시작합니다. 현재 자판기 ID: " + myVendingMachineId_);
        changeState(ControllerState::SYSTEM_READY);
    }

    // 메인 루프: 상태에 따라 작업을 수행하고, 사용자 입력을 기다리거나,
    // 네트워크 이벤트를 폴링합니다.
    while (currentState_ != ControllerState::SYSTEM_HALTED_REQUEST) {
        // 현재 상태에 따른 작업 수행
        processCurrentState();

        // 네트워크 이벤트 처리 (짧게 폴링하여 다른 작업 방해 최소화)
        // 사용자 입력 대기 상태가 아닌 경우에만 폴링하거나,
        // 또는 입력 대기 중에도 주기적으로 폴링할 수 있으나 CLI에서는 복잡해짐.
        // 여기서는 상태 처리 후 항상 약간의 이벤트 처리 시간을 줌.
        if (currentState_ != ControllerState::SYSTEM_HALTED_REQUEST) {
            // io_context_.poll()은 논블로킹으로, 처리할 핸들러가 없으면 즉시 반환.
            // io_context_.run_one()은 하나의 핸들러를 실행하거나, 없으면 즉시 반환 (논블로킹).
            // 여러 번 호출하여 큐에 쌓인 여러 이벤트를 처리할 수 있도록 함.
            for (int i = 0; i < 5; ++i) { // 예: 최대 5개 이벤트 처리 시도
                if (ioContext_.stopped()) ioContext_.restart(); // 필요시 재시작
                ioContext_.poll_one();
                if (ioContext_.stopped() && currentState_ == ControllerState::SYSTEM_HALTED_REQUEST) break; // 종료 상태면 중단
            }
        }
        // 현재 상태가 사용자 입력을 기다리는 상태라면,
        // processCurrentState() 내부의 UI 호출에서 프로그램 흐름이 멈춰있을 것임.
        // 해당 UI 호출이 끝나고 리턴해야 이 루프의 다음 반복으로 진행됨.
    }
    userInterface_.displayMessage("자판기 시스템을 종료합니다.");
}

void UserProcessController::changeState(ControllerState newState) {
    // std::cout << "[DEBUG] StateChange: From " << static_cast<int>(currentState_)
    //           << " To " << static_cast<int>(newState) << std::endl;
    currentState_ = newState;
}

void UserProcessController::resetCurrentTransactionState() {
    currentActiveOrder_.reset();
    isCurrentOrderPrepayment_ = false;
    pendingDrinkSelection_.reset();
    availableOtherVmsForDrink_.clear();
    selectedTargetVmForPrepayment_.reset();
}

domain::Drink UserProcessController::getDrinkDetails(const std::string& drinkCode) {
    try {
        auto allDrinks = inventoryService_.getAllDrinkTypes();
        for (const auto& drink : allDrinks) {
            if (drink.getDrinkCode() == drinkCode) {
                return drink;
            }
        }
        auto errorInfo = errorService_.processOccurredError(ErrorType::DRINK_NOT_FOUND, "음료 코드(" + drinkCode + ")에 해당하는 음료를 찾을 수 없습니다.");
        state_handlingError(errorInfo);
        return domain::Drink();
    } catch (const std::exception& e) {
        auto errorInfo = errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "음료 정보 조회 중 예외: " + std::string(e.what()));
        state_handlingError(errorInfo);
        return domain::Drink();
    }
}

void UserProcessController::initializeSystemAndRegisterMessageHandlers() {
    messageService_.registerMessageHandler(network::Message::Type::REQ_STOCK,
        [this](const network::Message& msg){ boost::asio::post(ioContext_, [this, msg](){ this->onReqStockReceived(msg); }); });
    messageService_.registerMessageHandler(network::Message::Type::RESP_STOCK,
        [this](const network::Message& msg){ boost::asio::post(ioContext_, [this, msg](){ this->onRespStockReceived(msg); }); });
    messageService_.registerMessageHandler(network::Message::Type::REQ_PREPAY,
        [this](const network::Message& msg){ boost::asio::post(ioContext_, [this, msg](){ this->onReqPrepayReceived(msg); }); });
    messageService_.registerMessageHandler(network::Message::Type::RESP_PREPAY,
        [this](const network::Message& msg){ boost::asio::post(ioContext_, [this, msg](){ this->onRespPrepayReceived(msg); }); });

    messageService_.startReceivingMessages(); // MessageReceiver의 start() 호출 포함
}


// -- 유스케이스 단계별 상태 처리 메소드들 --
void UserProcessController::processCurrentState() {
    // std::cout << "[DEBUG] Processing State: " << static_cast<int>(currentState_) << std::endl;
    switch (currentState_) {
        // ... (기존 case 문들: SYSTEM_READY, DISPLAYING_MAIN_MENU, AWAITING_DRINK_SELECTION, ...) ...
        case ControllerState::SYSTEM_READY:
            changeState(ControllerState::DISPLAYING_MAIN_MENU);
            break;
        case ControllerState::DISPLAYING_MAIN_MENU:
            state_displayingMainMenu();
            break;
        case ControllerState::AWAITING_DRINK_SELECTION:
            state_awaitingDrinkSelection();
            break;
        case ControllerState::AWAITING_PAYMENT_CONFIRMATION:
            state_awaitingPaymentConfirmation();
            break;
        case ControllerState::PROCESSING_PAYMENT:
            state_processingPayment();
            break;
        case ControllerState::DISPENSING_DRINK:
            state_dispensingDrink();
            break;
        case ControllerState::BROADCASTING_STOCK_REQUEST:
            state_broadcastingStockRequest();
            break;
        case ControllerState::AWAITING_STOCK_RESPONSES:
            // 이 상태에서는 onRespStockReceived 콜백으로 availableOtherVmsForDrink_가 채워지거나,
            // 아래의 수동 타임아웃 로직이 동작합니다.
            // run() 루프의 ioContext_.poll_one()을 통해 콜백이 처리될 기회를 가집니다.
            if (timeout_start_time_) { // 타임아웃 타이머가 시작되었다면
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - *timeout_start_time_);
                if (elapsed >= current_timeout_duration_) {
                    timeout_start_time_.reset(); // 타임아웃 발생, 타이머 해제
                    if (availableOtherVmsForDrink_.empty()) {
                        // UC9 E2: "30초 이내에 아무런 응답 메시지가 오지 않는 경우 시스템은 에러메시지를 호출해 보여준다." [cite: 19]
                        userInterface_.displayNoOtherVendingMachineFound(pendingDrinkSelection_ ? pendingDrinkSelection_->getName() : "선택된 음료");
                        auto errorInfo = errorService_.processOccurredError(ErrorType::RESPONSE_TIMEOUT_FROM_OTHER_VM, "주변 자판기 재고 조회 (" + std::to_string(current_timeout_duration_.count()) + "초)");
                        state_handlingError(errorInfo);
                    } else {
                        // 타임아웃 되었지만, 그 안에 일부 응답이라도 왔으면 그걸로 진행
                        userInterface_.displayMessage("응답 시간이 초과되었으나, 수신된 정보로 가까운 자판기를 안내합니다.");
                        changeState(ControllerState::DISPLAYING_OTHER_VM_OPTIONS);
                    }
                } else {
                    // 타임아웃 시간이 아직 지나지 않음. run() 루프에서 io_context_.poll_one()으로 계속 응답 확인.
                    // 현재 상태 유지. (명시적으로 아무것도 안 함)
                    // userInterface_.displayMessage("."); // 대기 중임을 시각적으로 표시 (선택적)
                    // std::this_thread::sleep_for(std::chrono::milliseconds(100)); // CPU 사용 방지 (선택적)
                }
            }
            // onRespStockReceived 콜백에서 충분한 응답을 받거나 특정 조건 만족 시
            // DISPLAYING_OTHER_VM_OPTIONS 상태로 변경할 수도 있음 (현재는 타임아웃 또는 모든 응답 대기).
            break;

        case ControllerState::DISPLAYING_OTHER_VM_OPTIONS:
            state_displayingOtherVmOptions(); // 다음 구현 대상
            break;
        case ControllerState::ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION:
            if (timeout_start_time_) { // 타임아웃 타이머가 시작되었다면
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - *timeout_start_time_);
                if (elapsed >= current_timeout_duration_) {
                    timeout_start_time_.reset(); // 타임아웃 발생, 타이머 해제
                    // UC16의 응답 타임아웃에 대한 명시적 예외 처리는 없으나, 일반적인 네트워크 오류로 처리 가능
                    std::string targetVmName = selectedTargetVmForPrepayment_ ? selectedTargetVmForPrepayment_->getId() : "대상 자판기";
                    auto errorInfo = errorService_.processOccurredError(ErrorType::RESPONSE_TIMEOUT_FROM_OTHER_VM, targetVmName + "로부터 선결제 재고 확보 응답 없음 (" + std::to_string(current_timeout_duration_.count()) + "초)");
                    state_handlingError(errorInfo);
                }
            }
            // onRespPrepayReceived 콜백에서 DISPLAYING_AUTH_CODE_INFO 또는 HANDLING_ERROR로 상태 변경.
            break;
        case ControllerState::DISPLAYING_AUTH_CODE_INFO:
            state_displayingAuthCodeInfo(); // 다음 구현 대상
            break;
        case ControllerState::AWAITING_AUTH_CODE_INPUT_PROMPT:
            state_awaitingAuthCodeInputPrompt(); // 다음 구현 대상
            break;
        case ControllerState::TRANSACTION_COMPLETED_RETURN_TO_MENU:
            state_transactionCompletedReturnToMenu(); // 다음 구현 대상
            break;
        case ControllerState::HANDLING_ERROR:
            // state_handlingError 함수 내부에서 다음 상태로 전이 (주로 DISPLAYING_MAIN_MENU)
            // ErrorInfo의 resolutionLevel에 따라 동작.
            break;
        
        default:
            state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "알 수 없는 컨트롤러 상태: " + std::to_string(static_cast<int>(currentState_))));
            break;
    }
}


// --- 첫 번째 상태 구현: 메인 메뉴 표시 및 사용자 선택 처리 ---
void UserProcessController::state_displayingMainMenu() {
    resetCurrentTransactionState();
    timeout_start_time_.reset(); // 이전 타임아웃 해제
    userInterface_.displayMessage("\n--- 메인 메뉴 ---");

    // UC1: 음료 목록 조회 및 표시
    auto allDrinks = inventoryService_.getAllDrinkTypes(); // R1.1 [cite: 65]
    std::vector<domain::Inventory> currentVmInventory; // 현재는 빈 리스트 전달
    userInterface_.displayDrinkList(allDrinks, currentVmInventory);

    std::vector<std::string> menuOptions = {"1. 음료 선택", "2. 인증 코드로 음료 받기", "3. 종료"};
    userInterface_.displayMainMenu(menuOptions);

    int choice = userInterface_.getUserChoice(menuOptions.size());

    switch (choice) {
        case 1:
            changeState(ControllerState::AWAITING_DRINK_SELECTION);
            break;
        case 2:
            changeState(ControllerState::AWAITING_AUTH_CODE_INPUT_PROMPT);
            break;
        case 3:
            changeState(ControllerState::SYSTEM_HALTED_REQUEST);
            break;
        default:
            state_handlingError(errorService_.processOccurredError(ErrorType::INVALID_MENU_CHOICE, "메인 메뉴 선택 범위 초과"));
            break;
    }
}

// --- 음료 선택 및 로컬 재고 확인 (UC2, UC3) ---
void UserProcessController::state_awaitingDrinkSelection() {
    timeout_start_time_.reset(); // 이전 타임아웃 해제
    // UC2. 사용자 음료 선택 처리
    // UserInterface::selectDrink는 모든 음료 목록을 받고, 사용자로부터 음료 코드를 입력받음
    std::string drinkCode = userInterface_.selectDrink(inventoryService_.getAllDrinkTypes(), {});

    if (drinkCode.empty()) { // 사용자가 선택을 취소한 경우 (UI가 빈 문자열 반환 가정)
        userInterface_.displayMessage("음료 선택이 취소되었습니다.");
        changeState(ControllerState::DISPLAYING_MAIN_MENU);
        return;
    }

    // 선택된 음료의 상세 정보 가져오기
    pendingDrinkSelection_ = getDrinkDetails(drinkCode);
    if (!pendingDrinkSelection_ || pendingDrinkSelection_->getDrinkCode().empty()) {
        // getDrinkDetails 내부에서 오류 발생 시 이미 state_handlingError가 호출되고
        // pendingDrinkSelection_이 비어있거나 기본값일 것임.
        // 이 경우 추가적인 오류 메시지 없이 메인 메뉴로 돌아가도록 유도 (state_handlingError에서 처리)
        return; // 오류 처리는 getDrinkDetails 또는 state_handlingError에서 담당
    }

    // UC3. 현재 자판기 재고 확인
    // R1.3. 현재 자판기의 재고를 확인한다. [cite: 65]
    auto availability = inventoryService_.checkDrinkAvailabilityAndPrice(drinkCode);

    if (availability.isAvailable) {
        userInterface_.displayMessage(pendingDrinkSelection_->getName() + " 선택됨. 가격: " + std::to_string(availability.price) + "원. 재고: " + std::to_string(availability.currentStock) + "개. 바로 결제합니다.");
        currentActiveOrder_ = orderService_.createOrder(myVendingMachineId_, *pendingDrinkSelection_);
        isCurrentOrderPrepayment_ = false;
        changeState(ControllerState::AWAITING_PAYMENT_CONFIRMATION);
    } else {
        // UC3: "재고가 없거나 이거나 판매대상이 아닌 경우, 시스템은 BroadcastMessage 준비 후 인근 자판기에서의 탐색정보로 전환한다(UC8)" [cite: 8]
        userInterface_.displayOutOfStockMessage(pendingDrinkSelection_->getName());
        // UC8의 전제조건: 사용자가 선택한 음료가 현재 자판기에서 판매 불가 혹은 재고 없음 상태여야 함 [cite: 18]
        changeState(ControllerState::BROADCASTING_STOCK_REQUEST);
    }
}


// --- 다음 상태: 결제 요청 및 처리 (UC4, UC5, UC6) ---
void UserProcessController::state_awaitingPaymentConfirmation() {
    timeout_start_time_.reset(); // 이전 타임아웃 해제
    if (!currentActiveOrder_ || !pendingDrinkSelection_) {
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "결제 확인 단계에서 주문 또는 음료 정보 없음"));
        return;
    }

    // OrderService에서 생성된 Order 객체에는 가격 정보가 직접적으로는 없음.
    // Drink 객체에서 가격을 가져오거나, InventoryService::checkDrinkAvailabilityAndPrice에서 얻은 가격 사용.
    // 여기서는 pendingDrinkSelection_ (domain::Drink)에 가격이 있다고 가정.
    int price = pendingDrinkSelection_->getPrice();

    if (isCurrentOrderPrepayment_) {
        // UC11. 선결제 요청
        userInterface_.displayMessage(pendingDrinkSelection_->getName() + "을(를) 다른 자판기(" + (selectedTargetVmForPrepayment_ ? selectedTargetVmForPrepayment_->getId() : "대상 자판기") + ")에서 수령하기 위해 선결제를 진행합니다. 가격: " + std::to_string(price) + "원.");
    } else {
        userInterface_.displayMessage(pendingDrinkSelection_->getName() + "을(를) 현재 자판기에서 구매합니다. 가격: " + std::to_string(price) + "원.");
    }

    // UC4. 사용자 결제 요청
    userInterface_.displayPaymentPrompt(price); // "카드를 삽입해주세요" 등의 메시지 표시

    // 사용자에게 결제 진행 여부 확인 (Y/N)
    // UserInterface에 bool confirmPayment() 와 같은 메소드가 있다고 가정.
    // UserInterface.hpp에 bool confirmPayment(); 추가 필요
    bool paymentConfirmed = userInterface_.confirmPayment();

    if (paymentConfirmed) {
        changeState(ControllerState::PROCESSING_PAYMENT);
    } else {
        userInterface_.displayMessage("결제가 취소되었습니다.");
        // 현재 주문이 있다면 취소 처리 또는 상태 변경 (선택적)
        // if(currentActiveOrder_) { orderService_.cancelOrder(*currentActiveOrder_); }
        changeState(ControllerState::DISPLAYING_MAIN_MENU);
    }
}

void UserProcessController::state_processingPayment() {
    timeout_start_time_.reset(); // 이전 타임아웃 해제
    // UC4. 카드 결제 시도 및 외부 시스템 응답 대기
    userInterface_.displayPaymentProcessing(); // "결제 진행 중..."

    // 외부 결제 시스템(시뮬레이션) 호출
    network::PaymentCallbackReceiver paymentSim; // 멤버 변수로 두거나 주입받는 것이 좋으나, 현재는 로컬 사용
    bool paymentSuccess = false;

    // PaymentCallbackReceiver::simulatePrepayment는 콜백을 사용.
    // 순차적 흐름을 위해, 콜백이 즉시 실행되거나, 이 함수 내에서 콜백 완료를 기다리는 방식으로 처리.
    // 현재 simulatePrepayment는 내부 sleep 후 콜백을 직접 호출하므로, 이 함수는 콜백 실행까지 대기.
    paymentSim.simulatePrepayment([this, &paymentSuccess](bool success) {
        paymentSuccess = success;
    }, 3); // 3초 지연 후 콜백 실행 (성공률은 10%로 수정되었었음)

    if (paymentSuccess) {
        userInterface_.displayPaymentResult(true, "결제 성공!");
        // UC5. 결제 승인 처리
        if (currentActiveOrder_) { // currentActiveOrder_가 유효한지 확인
            orderService_.processOrderApproval(*currentActiveOrder_, isCurrentOrderPrepayment_); //
            if (isCurrentOrderPrepayment_) {
                // 선결제 성공 시, UC16(재고 확보 요청) & UC12(인증코드 발급) 로직으로 연결
                if (!selectedTargetVmForPrepayment_) {
                    state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "선결제 대상 자판기 정보가 선택되지 않았습니다."));
                    return;
                }
                changeState(ControllerState::ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION);
            } else {
                changeState(ControllerState::DISPENSING_DRINK);
            }
        } else {
             state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "결제 성공 후 처리 중 주문 정보 없음."));
        }
    } else {
        // UC4 Exceptional E1: (30초 응답 없을 시) - 현재 시뮬레이션은 3초 후 무조건 응답. 타임아웃은 PAYMENT_PROCESSING_FAILED로 통합.
        // UC4 Exceptional E2: (승인 메시지 손상) - 현재 시뮬레이션은 단순 성공/실패.
        userInterface_.displayPaymentResult(false, "결제 실패.");
        // UC6. 결제 거절 처리
        if (currentActiveOrder_) { // currentActiveOrder_가 유효한지 확인
            orderService_.processOrderDeclination(*currentActiveOrder_); //
        }
        changeState(ControllerState::DISPLAYING_MAIN_MENU);
    }
}


// --- 음료 배출 (UC7) ---
void UserProcessController::state_dispensingDrink() {
    timeout_start_time_.reset(); // 이전 타임아웃 해제
    if (!pendingDrinkSelection_ || !currentActiveOrder_) {
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "음료 배출 단계에서 음료 또는 주문 정보가 없습니다."));
        return;
    }

    // UC7. 음료 배출 제어
    userInterface_.displayDispensingDrink(pendingDrinkSelection_->getName()); // "음료 배출 중..."

    // 실제 배출 하드웨어 제어 로직 (시뮬레이션)
    std::this_thread::sleep_for(std::chrono::seconds(2)); // 배출 시간 시뮬레이션

    userInterface_.displayDrinkDispensed(pendingDrinkSelection_->getName()); // "음료가 나왔습니다." [cite: 16] (메시지 표시)

    // 일반 구매 시 재고 차감은 OrderService::processOrderApproval에서 이미 처리됨.
    // 선결제 음료 수령 완료 시 (인증 코드 사용 완료)
    if (isCurrentOrderPrepayment_) {
        // UC14 (인증코드 유효성 검증)의 마지막 단계: "(S): 일치할 경우 AuthCode를 만료시킨다." [cite: 30]
        prepaymentService_.changeAuthCodeStatusToUsed(currentActiveOrder_->getCertCode());
    }

    changeState(ControllerState::TRANSACTION_COMPLETED_RETURN_TO_MENU);
}

// --- 현재 자판기 재고 없을 시 다른 자판기 조회 시작 (UC8) ---
void UserProcessController::state_broadcastingStockRequest() {
    if (!pendingDrinkSelection_) {
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "재고 조회 요청 단계에서 선택된 음료 정보가 없습니다."));
        return;
    }

    // UC8. 재고 조회 브로드캐스트
    userInterface_.displayMessage(pendingDrinkSelection_->getName() + " 재고를 주변 자판기에 문의합니다...");
    messageService_.sendStockRequestBroadcast(pendingDrinkSelection_->getDrinkCode()); // [cite: 18] "(S): req_stock타입의 메시지를 주변 Vending Machine 들에게 전송한다."
    availableOtherVmsForDrink_.clear(); // 이전 다른 자판기 조회 결과 초기화

    // 타임아웃 설정 (UC9 E2: "30초 이내에 아무런 응답 메시지가 오지 않는 경우 시스템은 에러메시지를 호출해 보여준다." [cite: 19])
    timeout_start_time_ = std::chrono::steady_clock::now();
    current_timeout_duration_ = std::chrono::seconds(30); // 디자인 문서 기준 30초

    changeState(ControllerState::AWAITING_STOCK_RESPONSES);
    // 다음 processCurrentState() 호출 시 AWAITING_STOCK_RESPONSES case에서 타임아웃 및 응답 처리
}



void UserProcessController::state_awaitingStockResponses() { /* ... processCurrentState에서 일부 처리 ... */ }

    // --- 다른 자판기 옵션 표시 및 선결제 여부 확인 (UC9, UC10, UC11) ---
    void UserProcessController::state_displayingOtherVmOptions() {
        timeout_start_time_.reset(); // 이전 타임아웃 해제

        if (!pendingDrinkSelection_) {
            state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "가까운 자판기 안내 단계에서 음료 정보 없음."));
            return;
        }

        if (availableOtherVmsForDrink_.empty()) {
            // 이 경우는 AWAITING_STOCK_RESPONSES에서 타임아웃 처리되었거나, 모든 응답이 재고 없음이었을 경우.
            userInterface_.displayNoOtherVendingMachineFound(pendingDrinkSelection_->getName());
            changeState(ControllerState::DISPLAYING_MAIN_MENU);
            return;
        }

        // UC10. 가장 가까운 자판기 안내
        // R3.2. 응답받은 재고 정보 중, 사용자에게 가장 가까운 자판기의 위치를 안내한다. [cite: 68]
        try {
            // DistanceService의 findNearestAvailableVendingMachine 사용
            // 이 메소드는 std::vector<OtherVendingMachineInfo> (id, coordX, coordY, hasStock 포함)를 받고,
            // 계산된 거리를 포함하여 정렬 후, 가장 적합한 자판기의 domain::VendingMachine을 반환.
            std::optional<domain::VendingMachine> nearestVm = distanceService_.findNearestAvailableVendingMachine(
                myVendingMachineX_, myVendingMachineY_, availableOtherVmsForDrink_
            );

            if (nearestVm) {
                selectedTargetVmForPrepayment_ = nearestVm; // 선결제 대상 자판기로 저장
                userInterface_.displayNearestVendingMachine(*selectedTargetVmForPrepayment_, pendingDrinkSelection_->getName()); //

                // UC11. 선결제 요청
                bool confirmPrepay = userInterface_.confirmPrepayment(pendingDrinkSelection_->getName()); // "선결제 하시겠습니까? (Y/N)"
                if (confirmPrepay) {
                    // 선결제를 위한 Order 객체 생성 또는 기존 Order 사용 (여기서는 새로 생성)
                    // R4.1. 사용자가 현재 자판기에서 타 자판기의 음료를 선결제할 수 있도록 한다. [cite: 69]
                    currentActiveOrder_ = orderService_.createOrder(myVendingMachineId_, *pendingDrinkSelection_);
                    isCurrentOrderPrepayment_ = true; // 선결제 플래그 설정
                    changeState(ControllerState::AWAITING_PAYMENT_CONFIRMATION); // 결제 단계로 이동
                } else {
                    userInterface_.displayMessage("선결제가 취소되었습니다.");
                    changeState(ControllerState::DISPLAYING_MAIN_MENU);
                }
            } else {
                // 재고가 있는 자판기를 찾았으나, DistanceService에서 가장 가까운 자판기를 결정하지 못한 경우 (로직 오류 등)
                userInterface_.displayNoOtherVendingMachineFound(pendingDrinkSelection_->getName());
                state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "가까운 자판기를 찾았으나 선택 오류."));
            }

        } catch (const std::runtime_error& e) { // findNearestAvailableVendingMachine이 후보 없을 때 던지는 예외
            userInterface_.displayNoOtherVendingMachineFound(pendingDrinkSelection_->getName());
            state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "가까운 자판기 찾는 중 오류: " + std::string(e.what())));
        } catch (const std::exception& e) { // 기타 예외 (ID 파싱 등)
            state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "가까운 자판기 처리 중 예외: " + std::string(e.what())));
        }
    }
void UserProcessController::state_awaitingPrepaymentChoice() { /* ... 구현 필요 ... */ }
// --- 선결제 후 인증코드 발급 및 재고 확보 요청 (UC12, UC16) ---
void UserProcessController::state_issuingAuthCodeAndRequestingReservation() {
    if (!currentActiveOrder_ || !pendingDrinkSelection_ || !selectedTargetVmForPrepayment_ || !isCurrentOrderPrepayment_) {
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드 발급/재고 확보 요청 단계에서 정보 부족."));
        return;
    }
    // currentActiveOrder_에는 processOrderApproval에서 이미 인증코드가 생성되어 있어야 함 (setCertCode 호출됨)
    if (currentActiveOrder_->getCertCode().empty()) {
         state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "선결제 주문에 인증 코드가 생성되지 않았습니다."));
        return;
    }

    // UC16. 재고 확보 요청 전송
    // R4.1, R4.2, R4.3 관련 [cite: 34]
    userInterface_.displayMessage(selectedTargetVmForPrepayment_->getId() + " 자판기에 " + pendingDrinkSelection_->getName() + " 재고 확보를 요청합니다... (인증코드: " + currentActiveOrder_->getCertCode() + ")");
    messageService_.sendPrepaymentReservationRequest(
        selectedTargetVmForPrepayment_->getId(),
        currentActiveOrder_->getDrinkCode(),
        currentActiveOrder_->getCertCode()
    ); //

    // 타임아웃 설정 (예: 30초)
    timeout_start_time_ = std::chrono::steady_clock::now();
    current_timeout_duration_ = std::chrono::seconds(30); // 선결제 재고 확보 응답 대기 시간

    // 상태는 현재 그대로 ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION 유지,
    // onRespPrepayReceived 콜백 또는 아래 processCurrentState의 타임아웃 로직에서 다음 상태로 변경.
}

    // --- 인증코드 및 위치 안내 (UC12) ---
    void UserProcessController::state_displayingAuthCodeInfo() {
        timeout_start_time_.reset(); // 이전 타임아웃 해제

        if (!currentActiveOrder_ || currentActiveOrder_->getCertCode().empty() || !selectedTargetVmForPrepayment_ || !pendingDrinkSelection_) {
            state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드 안내 단계에서 정보 부족."));
            return;
        }

        // UC12. 인증코드 발급 및 위치 안내
        // R4.2. 선결제 완료 시 인증 코드를 생성하고 출력한다. [cite: 69] (생성은 PrepaymentService, 저장은 OrderService에서 완료됨)
        // R4.3. 사용자가 다른 자판기에 가서 발급받은 인증코드를 입력하면 음료를 제공해야 한다. (다음 단계) [cite: 84]
        // (요구사항 문서) 선결제를 마친 후 사용자에게 인증코드와 선결제를 마친 자판기의 위치 정보(좌표) 등 필요한 정보를 안내해야 한다. [cite: 86]
        userInterface_.displayAuthCode(
            currentActiveOrder_->getCertCode(), // UC12: "(S): AuthCode를 사용자에게 표시한다." [cite: 25]
            *selectedTargetVmForPrepayment_,    // 위치 정보 포함
            pendingDrinkSelection_->getName()
        );

        changeState(ControllerState::TRANSACTION_COMPLETED_RETURN_TO_MENU);
    }
    // --- 인증코드 입력 프롬프트 및 처리 (UC13, UC14) ---
    void UserProcessController::state_awaitingAuthCodeInputPrompt() {
        timeout_start_time_.reset(); // 이전 타임아웃 해제
        // UC13. 인증코드 입력 처리
        std::string authCode = userInterface_.getAuthCodeInput(); // "(A): 사용자가 자판기의 인증코드 입력 화면으로 이동한다. (A): 사용자가 인증코드를 입력한다." [cite: 28]

        if (authCode.empty()) { // 사용자가 입력을 취소한 경우 (UI가 빈 문자열 반환 가정)
            userInterface_.displayMessage("인증 코드 입력이 취소되었습니다.");
            changeState(ControllerState::DISPLAYING_MAIN_MENU);
            return;
        }

        // UC13: "(S): 입력 문자열이 정규식 “[A-Za-z0-9]{5}""와 일치하는지 검사한다." [cite: 28]
        if (!prepaymentService_.isValidAuthCodeFormat(authCode)) {
            // UC13 E1: "형식이 맞지 않는 코드가 입력되면 시스템은 에러메시지를 호출해 보여준다." [cite: 28]
            auto errorInfo = errorService_.processOccurredError(ErrorType::AUTH_CODE_INVALID_FORMAT, authCode);
            state_handlingError(errorInfo); // state_handlingError는 RETRY_INPUT 시 메인으로 보냄.
                                            // 여기서 직접 AWAITING_AUTH_CODE_INPUT_PROMPT로 보내려면 수정 필요.
                                            // 현재는 메인 메뉴로 돌아간 후 다시 시도해야 함.
            return;
        }

        // UC14. 인증코드 유효성 검증
        // "(S): 코드가 유효한 형식일 경우, 시스템은 “인증코드 유효성 검증 및 음료 제공”(UC15->실제로는 UC14) 으로 흐름을 전환한다." [cite: 28]
        domain::PrePaymentCode prepayDetails = prepaymentService_.getPrepaymentDetailsIfActive(authCode); // "(S): 저장하고 있는 코드...와 입력 코드가 일치하는지 확인한다." [cite: 30]

        if (prepayDetails.getCode().empty()) { // 오류 발생 (getPrepaymentDetailsIfActive 내부에서 ErrorService 호출됨)
            // UC14 E1: "이미 사용된 인증코드이거나 시스템에 저장되 있지 않은 인증코드이면, 시스템은 에러메시지를 호출해 보여준다." [cite: 30]
            // 오류 메시지는 ErrorService에서 생성, state_handlingError에서 표시 후 메인 메뉴로.
            // state_handlingError(errorService_.processOccurredError(ErrorType::AUTH_CODE_NOT_FOUND / AUTH_CODE_ALREADY_USED ...)); // 이미 내부에서 호출됨
            changeState(ControllerState::DISPLAYING_MAIN_MENU); // getPrepaymentDetailsIfActive에서 오류 처리 후 기본 객체 반환 시
            return;
        }

        // 인증 코드 유효함. 연결된 주문 정보 가져오기
        // 인증 코드 유효함. 연결된 주문 정보 가져오기
        std::shared_ptr<domain::Order> heldOrderPtr = prepayDetails.getHeldOrder(); // PrePaymentCode에 연결된 Order 정보

        if (heldOrderPtr) { // shared_ptr가 유효한 객체를 가리키는지 확인
            currentActiveOrder_ = *heldOrderPtr; // 포인터가 가리키는 실제 Order 객체를 복사하여 할당
        } else {
            // heldOrderPtr이 nullptr인 경우 (PrePaymentCode에 연결된 Order가 없는 비정상 상황)
            state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드에 연결된 주문 객체가 유효하지 않습니다."));
            return;
        }

        if (!currentActiveOrder_) { // 할당 후에도 currentActiveOrder_가 여전히 비어있는 경우 (위에서 이미 처리됨)
            state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드에 연결된 주문 정보가 없습니다."));
            return;
        }
        // 인증 코드로 음료 정보 가져오기
        pendingDrinkSelection_ = getDrinkDetails(currentActiveOrder_->getDrinkCode());
        if (!pendingDrinkSelection_ || pendingDrinkSelection_->getDrinkCode().empty()) {
            state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드의 음료 정보를 찾을 수 없습니다: " + currentActiveOrder_->getDrinkCode()));
            return;
        }
        
        isCurrentOrderPrepayment_ = true; // 인증 코드로 받는 것은 선결제된 음료임
        userInterface_.displayMessage("인증 코드 (" + authCode + ") 확인 완료: " + pendingDrinkSelection_->getName());
        // UC14: "(S): UC7 음료 배출 단계로 이동한다." [cite: 30]
        changeState(ControllerState::DISPENSING_DRINK);
    }

    // --- 거래 완료 후 메인 메뉴로 ---
    void UserProcessController::state_transactionCompletedReturnToMenu() {
        timeout_start_time_.reset(); // 이전 타임아웃 해제
        userInterface_.displayMessage("거래가 완료되었습니다. 감사합니다.");
        resetCurrentTransactionState(); // 현재 거래 관련 정보 초기화
        changeState(ControllerState::DISPLAYING_MAIN_MENU);
    }


    void UserProcessController::state_handlingError(const service::ErrorInfo& errorInfo) {
        timeout_start_time_.reset(); // 오류 발생 시 진행 중이던 타임아웃 해제
        userInterface_.displayError(errorInfo.userFriendlyMessage);
        switch (errorInfo.resolutionLevel) {
            case ErrorResolutionLevel::RETRY_INPUT:
                userInterface_.displayMessage("입력을 다시 시도해주세요. (메인 메뉴로 돌아갑니다)"); // 현재는 단순화
                changeState(ControllerState::DISPLAYING_MAIN_MENU);
                break;
            case ErrorResolutionLevel::RETURN_TO_MAIN_MENU:
            default:
                changeState(ControllerState::DISPLAYING_MAIN_MENU);
                break;
        }
    }
// --- MessageService로부터 호출될 콜백 핸들러들 ---
// 중요: 이 콜백들은 io_context 스레드(또는 poll()을 호출한 스레드)에서 실행될 수 있음.
// UserProcessController의 멤버 변수 접근 시 동기화 문제 발생 가능성 있음 (단일 스레드 모델에서는 괜찮음).
// 현재는 단일 스레드에서 io_context_.poll_one()을 통해 순차적으로 처리되므로 동기화 문제 적음.
// 콜백 내에서 UI 직접 호출은 CLI 환경에서는 문제 없을 수 있으나, GUI에서는 스레드 문제 유의.
// boost::asio::post(ioContext_, [this, ...]{ /* UI 작업 또는 상태 변경 */ }); 로 메인 스레드 큐에 작업을 올리는 것이 안전.
// 현재 initializeSystemAndRegisterMessageHandlers에서 post를 사용하도록 수정.

void UserProcessController::onReqStockReceived(const network::Message& msg) {
    // UC17. 타 자판기로부터 재고 조회 요청 수신 및 응답 처리
    std::cout << "[NETWORK] Received REQ_STOCK from " << msg.src_id << std::endl;
    if (msg.msg_content.count("item_code")) {
        std::string requestedDrinkCode = msg.msg_content.at("item_code");
        auto availability = inventoryService_.checkDrinkAvailabilityAndPrice(requestedDrinkCode);
        int stockCount = 0;
        if(availability.isAvailable) {
            // InventoryService가 정확한 재고량을 반환하도록 수정 필요.
            // 임시로, 재고 있으면 1, 없으면 0.
            // PFR 표2에 따르면 item_num은 음료 개수(0~99) [cite: 105]
            // 여기서는 hasStock으로 재고 있으면 1개 있다고 가정.
            // 정확한 재고량 제공하려면 InventoryService.checkDrinkAvailabilityAndPrice 수정 필요.
           auto availabilityInfo = inventoryService_.checkDrinkAvailabilityAndPrice(requestedDrinkCode);
            int stockCount = availabilityInfo.currentStock; // InventoryService를 통해 얻은 정확한 재고량 사용
            // messageService_.sendStockResponse(...) 호출 시 stockCount 전달
        }

        messageService_.sendStockResponse(msg.src_id, requestedDrinkCode, stockCount);
    } else {
        auto errorInfo = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "REQ_STOCK 메시지 (item_code 누락 from " + msg.src_id + ")");
        // 이 오류는 사용자에게 직접 보이기보다는 로깅 또는 무시 대상일 수 있음.
        userInterface_.displayError("[내부 오류] " + errorInfo.userFriendlyMessage); // 임시로 UI에 표시
    }
}

void UserProcessController::onRespStockReceived(const network::Message& msg) {
    // UC9. 재고 조회 브로드캐스트에 대한 다른 자판기들의 응답 수신 [cite: 19]
    // std::cout << "[NETWORK] Received RESP_STOCK from " << msg.src_id << std::endl; // 디버깅용

    // AWAITING_STOCK_RESPONSES 상태일 때만 유효한 응답으로 처리
    if (currentState_ != ControllerState::AWAITING_STOCK_RESPONSES) {
        // std::cout << "[NETWORK] Ignoring RESP_STOCK, current state is not AWAITING_STOCK_RESPONSES. State: " << static_cast<int>(currentState_) << std::endl;
        return;
    }

    try {
        std::string drinkCode = msg.msg_content.at("item_code");
        int stockQty = std::stoi(msg.msg_content.at("item_num"));
        int x = std::stoi(msg.msg_content.at("coor_x"));
        int y = std::stoi(msg.msg_content.at("coor_y"));
        std::string vmId = msg.src_id;

        // 현재 요청 중인 음료에 대한 응답이고, 재고가 있는 경우에만 리스트에 추가
        if (pendingDrinkSelection_ && pendingDrinkSelection_->getDrinkCode() == drinkCode && stockQty > 0) {
            // 중복 응답 방지 (이미 리스트에 있는 vmId인지 확인) - 선택적
            bool already_received = false;
            for(const auto& ovm_info : availableOtherVmsForDrink_){
                if(ovm_info.id == vmId) { // DistanceService.hpp의 OtherVendingMachineInfo 구조체 사용 가정
                    already_received = true;
                    break;
                }
            }
            if(already_received){
                // std::cout << "[NETWORK] Ignoring duplicate RESP_STOCK from " << vmId << std::endl;
                return;
            }

            availableOtherVmsForDrink_.push_back({vmId, x, y, true}); // DistanceService.hpp의 OtherVendingMachineInfo 사용
            userInterface_.displayMessage(vmId + " 자판기에 " + pendingDrinkSelection_->getName() + " 재고(" + std::to_string(stockQty) + "개) 확인됨.");

            // 나를 제외한 7개의 모든 응답을 받으면 타임아웃 기다리지 않고 바로 진행
            // total_other_vms_ 멤버 변수를 사용 
            if (availableOtherVmsForDrink_.size() >= total_other_vms_) {
                timeout_start_time_.reset(); // 타임아웃 메커니즘 중지
                userInterface_.displayMessage("모든 주변 자판기로부터 응답을 수신했습니다.");
                changeState(ControllerState::DISPLAYING_OTHER_VM_OPTIONS);
                // 다음 processCurrentState() 호출 시 해당 상태 로직 실행
            }
        }
    } catch (const std::exception& e) {
        // UC9 E1: "응답 메시지가 손상되었거나 유효하지 않은 경우, 해당 응답은 무시된다." [cite: 19]
        // 오류 메시지를 ErrorService를 통해 보고할 수 있으나, 사용자에게 직접적인 영향은 없을 수 있음 (해당 응답 무시)
        auto errorInfo = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "RESP_STOCK 메시지 처리 오류 (from " + msg.src_id + "): " + e.what());
        // userInterface_.displayError("[내부 오류] " + errorInfo.userFriendlyMessage); // 디버깅 시에만 표시
        std::cerr << "[ERROR] Invalid RESP_STOCK message from " << msg.src_id << ": " << e.what() << std::endl;
    }
}


void UserProcessController::onReqPrepayReceived(const network::Message& msg) {
    // UC15. 선결제요청 수신 및 재고 확보 [cite: 32]
    std::cout << "[NETWORK] Received REQ_PREPAY from " << msg.src_id << std::endl;
    try {
        std::string drinkCode = msg.msg_content.at("item_code");
        // int itemNum = std::stoi(msg.msg_content.at("item_num")); // 보통 1
        std::string certCode = msg.msg_content.at("cert_code");
        std::string requestingVmId = msg.src_id;

        bool available = false;
        auto availabilityInfo = inventoryService_.checkDrinkAvailabilityAndPrice(drinkCode);
        if (availabilityInfo.isAvailable) {
            inventoryService_.decreaseStock(drinkCode); // 재고 확보 (차감)
            prepaymentService_.recordIncomingPrepayment(certCode, drinkCode, requestingVmId); // 인증코드 등록
            available = true;
            userInterface_.displayMessage(requestingVmId + "로부터 " + drinkCode + " 선결제 요청 처리 완료 (인증코드: " + certCode + ")");
        } else {
            // UC15 Alternative A1: "Drink.stock 이 0이면 ""PREPAY_REJECTED"" 메시지를 송신한다." -> available = F [cite: 32]
            userInterface_.displayMessage(requestingVmId + "로부터 " + drinkCode + " 선결제 요청 처리 실패 (재고 없음)");
        }
        messageService_.sendPrepaymentReservationResponse(requestingVmId, drinkCode, available);

    } catch (const std::exception& e) {
        auto errorInfo = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "REQ_PREPAY 메시지 (from " + msg.src_id + "): " + e.what());
        userInterface_.displayError("[내부 오류] " + errorInfo.userFriendlyMessage);
    }
}

void UserProcessController::onRespPrepayReceived(const network::Message& msg) {
    // UC16. 재고 확보 요청 전송 후 응답 수신 [cite: 34]
    // std::cout << "[NETWORK] Received RESP_PREPAY from " << msg.src_id << std::endl;
    if (currentState_ != ControllerState::ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION) {
        // std::cout << "[NETWORK] Ignoring RESP_PREPAY, current state is " << static_cast<int>(currentState_) << std::endl;
        return;
    }
    timeout_start_time_.reset(); // 응답을 받았으므로 타임아웃 중지!

    try {
        // ... (기존 onRespPrepayReceived 로직) ...
        std::string drinkCode = msg.msg_content.at("item_code");
        std::string availability = msg.msg_content.at("availability"); // "T" or "F"

        if (pendingDrinkSelection_ && pendingDrinkSelection_->getDrinkCode() == drinkCode &&
            currentActiveOrder_ && !currentActiveOrder_->getCertCode().empty()) {
            if (availability == "T") {
                // UC16: "resp_prepay응답을 수신하면 인증코드 발급 및 위치 안내 단계를 수행한다 (UC12)" [cite: 34]
                changeState(ControllerState::DISPLAYING_AUTH_CODE_INFO);
            } else {
                // UC16 E1: "응답 msg_content.availability == F 시 시스템은 에러메시지를 호출해 보여준다." [cite: 34]
                auto errorInfo = errorService_.processOccurredError(ErrorType::STOCK_RESERVATION_FAILED_AT_OTHER_VM,
                                                                  selectedTargetVmForPrepayment_ ? selectedTargetVmForPrepayment_->getId() : "대상 자판기");
                state_handlingError(errorInfo);
            }
        } else {
            auto errorInfo = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "RESP_PREPAY 메시지 (음료 코드 불일치 또는 주문 정보 없음 from " + msg.src_id + ")");
            state_handlingError(errorInfo);
        }
    } catch (const std::exception& e) {
        auto errorInfo = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "RESP_PREPAY 메시지 처리 오류 (from " + msg.src_id + "): " + e.what());
        state_handlingError(errorInfo);
    }
}

}
 // namespace service