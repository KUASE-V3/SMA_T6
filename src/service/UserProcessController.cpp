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

#include <boost/asio/post.hpp>
#include <boost/asio/bind_executor.hpp> // strand 사용 고려 시

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <thread>   // std::this_thread::sleep_for (음료 배출 등 간단한 동기 지연)
#include <mutex>
#include <condition_variable>
#include <iostream> 
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
    int myVmY,
    int totalOtherVmCount
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
    total_other_vms_(totalOtherVmCount), // 주입받은 값으로 초기화
    response_timer_(ioContext) {
}

void UserProcessController::run() {
    { // 초기화는 메인 스레드에서, io_context 스레드 시작 전 수행
        if (currentState_ == ControllerState::INITIALIZING) {
            initializeSystemAndRegisterMessageHandlers(); // 내부에서 콜백 등록
            userInterface_.displayMessage("자판기 시스템을 시작합니다. 현재 자판기 ID: " + myVendingMachineId_);
            // changeState는 뮤텍스를 사용하므로 여기서 호출 가능
            changeState(ControllerState::SYSTEM_READY);
        }
    }

    while (true) {
        ControllerState stateToExecute;
        { // 현재 상태를 읽기 위해 뮤텍스 사용
            std::lock_guard<std::mutex> lock(mtx_);
            if (currentState_ == ControllerState::SYSTEM_HALTED_REQUEST) {
                break; // 루프 종료
            }
            stateToExecute = currentState_;
        }
        processCurrentState(); // 각 상태 처리 (내부에서 필요시 뮤텍스 및 CV 사용)
    }
    userInterface_.displayMessage("자판기 시스템을 종료합니다.");
}

void UserProcessController::changeState(ControllerState newState) {
    std::lock_guard<std::mutex> lock(mtx_); // currentState_ 보호
    currentState_ = newState;
}

void UserProcessController::resetCurrentTransactionState() {
    std::lock_guard<std::mutex> lock(mtx_); // 공유 멤버 변수 접근 보호
    currentActiveOrder_.reset();
    isCurrentOrderPrepayment_ = false;
    pendingDrinkSelection_.reset();
    availableOtherVmsForDrink_.clear();
    selectedTargetVmForPrepayment_.reset();
    response_timer_.cancel(); // 진행 중이던 Asio 타이머 취소
    cv_data_ready_ = false;   // 조건 변수 플래그 리셋
}

domain::Drink UserProcessController::getDrinkDetails(const std::string& drinkCode) {
    // InventoryService 내부에서 데이터 접근 동기화가 이루어진다고 가정.
    try {
        auto allDrinks = inventoryService_.getAllDrinkTypes();
        for (const auto& drink : allDrinks) {
            if (drink.getDrinkCode() == drinkCode) {
                return drink;
            }
        }
        { // 오류 정보 설정 및 상태 변경을 위해 뮤텍스 사용
            std::lock_guard<std::mutex> lock(mtx_);
            last_error_info_ = errorService_.processOccurredError(ErrorType::DRINK_NOT_FOUND, "음료 코드(" + drinkCode + ")가 메뉴에 없습니다.");
            currentState_ = ControllerState::HANDLING_ERROR;
        }
        cv_.notify_one(); // 메인 루프에 상태 변경 알림
        return domain::Drink(); // 빈 객체 반환
    } catch (const std::exception& e) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            last_error_info_ = errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "음료 정보 조회 중 시스템 오류: " + std::string(e.what()));
            currentState_ = ControllerState::HANDLING_ERROR;
        }
        cv_.notify_one();
        return domain::Drink();
    }
}

void UserProcessController::initializeSystemAndRegisterMessageHandlers() {
    // MessageService의 핸들러 등록. 콜백은 io_context 스레드에서 실행됨.
    // boost::asio::post를 사용하여 핸들러 실행을 io_context 큐에 올림.
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

// Asio 타이머 시작 헬퍼 함수
void UserProcessController::startResponseTimer(std::chrono::seconds duration, ControllerState stateToWatchOnTimeout) {
    response_timer_.expires_after(duration); // 타이머 만료 시간 설정
    // 타이머 만료 시 호출될 콜백 등록
    response_timer_.async_wait(
        [this, stateToWatchOnTimeout](const boost::system::error_code& ec) {
            this->handleTimeout(ec, stateToWatchOnTimeout);
        }
    );
}

// Asio 타이머 만료 시 호출될 콜백
void UserProcessController::handleTimeout(const boost::system::error_code& ec, ControllerState expectedStateDuringTimeout) {
    if (ec == boost::asio::error::operation_aborted) {
        return; // 타이머가 정상적으로 취소됨 (예: 응답을 시간 내에 받음)
    }
    // 타임아웃 발생 (ec가 operation_aborted가 아닌 경우)
    std::lock_guard<std::mutex> lock(mtx_); // 공유 자원 접근 보호
    if (currentState_ == expectedStateDuringTimeout) { // 타임아웃 발생 시점에도 여전히 해당 대기 상태인지 확인
        if (currentState_ == ControllerState::AWAITING_STOCK_RESPONSES) { // UC9 타임아웃
            if (availableOtherVmsForDrink_.empty()) { // UC9 E2
                // userInterface_.displayNoOtherVendingMachineFound 호출은 메인 스레드에서 하는 것이 안전
                last_error_info_ = errorService_.processOccurredError(ErrorType::RESPONSE_TIMEOUT_FROM_OTHER_VM, "주변 자판기 재고 조회");
            } else { // 일부 응답이라도 수신된 경우
                // userInterface_.displayMessage 호출은 메인 스레드에서
                currentState_ = ControllerState::DISPLAYING_OTHER_VM_OPTIONS; // UC10으로
            }
        } else if (currentState_ == ControllerState::ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION) { // UC16 타임아웃
            std::string targetVmId_str = selectedTargetVmForPrepayment_ ? selectedTargetVmForPrepayment_->getId() : "대상 자판기";
            last_error_info_ = errorService_.processOccurredError(ErrorType::RESPONSE_TIMEOUT_FROM_OTHER_VM, targetVmId_str + "로부터 선결제 응답 없음");
        }

        if(last_error_info_ && currentState_ == expectedStateDuringTimeout) { // 오류가 설정되었고, 아직 상태가 안바뀌었다면
             currentState_ = ControllerState::HANDLING_ERROR;
        }
        cv_data_ready_ = true; // 메인 스레드가 다음 동작을 하도록 알림
        cv_.notify_one();      // 대기 중인 메인 스레드(processCurrentState)를 깨움
    }
}

void UserProcessController::processCurrentState() {
    ControllerState stateToExecute;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        stateToExecute = currentState_;
    }

    switch (stateToExecute) {
        case ControllerState::SYSTEM_READY:
            changeState(ControllerState::DISPLAYING_MAIN_MENU);
            break;
        case ControllerState::DISPLAYING_MAIN_MENU:
            state_displayingMainMenu();
            break;
        case ControllerState::AWAITING_DRINK_SELECTION:
            state_awaitingDrinkSelection(); // 여기서 재고 없으면 BROADCASTING_STOCK_REQUEST 로 상태 변경
            break;
        case ControllerState::BROADCASTING_STOCK_REQUEST: // UC8: 메시지 전송 및 대기 상태로 전환
            state_broadcastingStockRequest(); // 이 함수 내부에서 메시지 전송, current_timeout_duration_ 설정, 타이머 시작, AWAITING_STOCK_RESPONSES로 상태 변경까지 수행
            // 여기서 break 후, 다음 루프에서 AWAITING_STOCK_RESPONSES case를 타게 됨
            break;
        case ControllerState::AWAITING_STOCK_RESPONSES: // UC9: 재고 조회 응답 대기
            {
                std::unique_lock<std::mutex> lock(mtx_);
                // current_timeout_duration_은 state_broadcastingStockRequest에서 30초로 설정되어 있어야 함
                std::chrono::seconds waitDuration = current_timeout_duration_ + std::chrono::seconds(1);
                if (cv_.wait_for(lock, waitDuration, [this]{ return cv_data_ready_; })) {
                    // 응답 또는 Asio 타이머에 의해 깨어남. 콜백에서 상태 변경됨.
                } else { 
                    // 타임아웃 발생. 현재 상태를 다시 확인하고, 필요시 오류 처리
                    std::lock_guard<std::mutex> guard(mtx_); // currentState_ 다시 확인 위해 잠금
                    if (currentState_ == ControllerState::AWAITING_STOCK_RESPONSES) { // 상태가 안 바뀌었다면
                        if (availableOtherVmsForDrink_.empty()) {
                            last_error_info_ = errorService_.processOccurredError(ErrorType::RESPONSE_TIMEOUT_FROM_OTHER_VM, "주변 자판기 재고 조회 (컨트롤러 cv_.wait_for 타임아웃)");
                            currentState_ = ControllerState::HANDLING_ERROR;
                        } else {
                            currentState_ = ControllerState::DISPLAYING_OTHER_VM_OPTIONS;
                        }
                    }
                }
                cv_data_ready_ = false;
            }
            break;
        case ControllerState::AWAITING_PAYMENT_CONFIRMATION:
            state_awaitingPaymentConfirmation();
            break;
        case ControllerState::PROCESSING_PAYMENT:
            state_processingPayment(); // 여기서 선결제 성공 시 ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION 로 상태 변경
            break;
        case ControllerState::ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION: // UC16: 선결제 예약 요청 및 응답 대기
            state_issuingAuthCodeAndRequestingReservation(); // 이 함수 내부에서 메시지 전송, current_timeout_duration_ 설정(10초), 타이머 시작. 상태는 변경하지 않음.
            {
                std::unique_lock<std::mutex> lock(mtx_);
                // current_timeout_duration_은 state_issuingAuthCodeAndRequestingReservation에서 10초로 설정되어 있어야 함
                std::chrono::seconds waitDuration = current_timeout_duration_ + std::chrono::seconds(1);
                 if (cv_.wait_for(lock, waitDuration, [this]{ return cv_data_ready_; })) {
                    // 응답 또는 Asio 타이머에 의해 깨어남. 콜백에서 상태 변경됨.
                } else { // cv_.wait_for 자체 타임아웃
                    std::lock_guard<std::mutex> guard(mtx_);
                    if (currentState_ == ControllerState::ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION) {
                        std::string targetVmId_str = selectedTargetVmForPrepayment_ ? selectedTargetVmForPrepayment_->getId() : "대상 자판기";
                        last_error_info_ = errorService_.processOccurredError(ErrorType::RESPONSE_TIMEOUT_FROM_OTHER_VM, targetVmId_str + "로부터 선결제 예약 응답 시간 초과 (컨트롤러 cv_.wait_for 타임아웃)");
                        currentState_ = ControllerState::HANDLING_ERROR;
                    }
                }
                cv_data_ready_ = false;
            }
            break;
        case ControllerState::DISPENSING_DRINK:
            state_dispensingDrink();
            break;
        case ControllerState::DISPLAYING_OTHER_VM_OPTIONS:
            state_displayingOtherVmOptions();
            break;
        case ControllerState::DISPLAYING_AUTH_CODE_INFO:
            state_displayingAuthCodeInfo();
            break;
        case ControllerState::AWAITING_AUTH_CODE_INPUT_PROMPT:
            state_awaitingAuthCodeInputPrompt();
            break;
        case ControllerState::TRANSACTION_COMPLETED_RETURN_TO_MENU:
            state_transactionCompletedReturnToMenu();
            break;
        case ControllerState::HANDLING_ERROR:
            {
                std::optional<ErrorInfo> errorToHandle;
                { std::lock_guard<std::mutex> lock(mtx_); errorToHandle = last_error_info_; last_error_info_.reset(); }
                if(errorToHandle){ state_handlingError(*errorToHandle); }
                else { changeState(ControllerState::DISPLAYING_MAIN_MENU); }
            }
            break;
        default:
            {
                std::lock_guard<std::mutex> lock(mtx_);
                last_error_info_ = errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "알 수 없는 컨트롤러 상태 값 또는 처리되지 않은 상태(default case): " + std::to_string(static_cast<int>(currentState_)));
                currentState_ = ControllerState::HANDLING_ERROR;
                cv_.notify_one();
            }
            break;
    }
}
// --- 각 유스케이스 상태 처리 함수들 ---
// (내부에서 공유 멤버 변수 접근 시 std::lock_guard<std::mutex> lock(mtx_); 사용)

// UC1: 음료 목록 조회 및 표시
void UserProcessController::state_displayingMainMenu() {
    resetCurrentTransactionState(); // 내부에서 뮤텍스 사용
    userInterface_.displayMessage("\n=========== Vending Machine Menu ===========");
    std::vector<domain::Drink> allDrinks = inventoryService_.getAllDrinkTypes(); // PFR R1.1
    userInterface_.displayDrinkList(allDrinks, {});

    std::vector<std::string> menuOptions = {
        "1. 음료 선택 (구매/다른 자판기 조회)",
        "2. 인증 코드로 음료 받기",
        "3. 시스템 종료"
    };
    userInterface_.displayMainMenu(menuOptions);
    int choice = userInterface_.getUserChoice(menuOptions.size()); // 블로킹 입력

    std::lock_guard<std::mutex> lock(mtx_); // 상태 변경 전 뮤텍스
    switch (choice) {
        case 1: currentState_ = ControllerState::AWAITING_DRINK_SELECTION; break; // UC2로
        case 2: currentState_ = ControllerState::AWAITING_AUTH_CODE_INPUT_PROMPT; break; // UC13으로
        case 3: currentState_ = ControllerState::SYSTEM_HALTED_REQUEST; break;
        default:
            last_error_info_ = errorService_.processOccurredError(ErrorType::INVALID_MENU_CHOICE, "메인 메뉴 선택");
            currentState_ = ControllerState::HANDLING_ERROR;
            break;
    }
}

// UC2: 사용자 음료 선택 처리 & UC3: 현재 자판기 재고 확인
void UserProcessController::state_awaitingDrinkSelection() {
    std::string drinkCode = userInterface_.selectDrink(inventoryService_.getAllDrinkTypes(), {}); // 블로킹 입력
    if (drinkCode.empty()) {
        userInterface_.displayMessage("음료 선택이 취소되었습니다.");
        changeState(ControllerState::DISPLAYING_MAIN_MENU);
        return;
    }

    domain::Drink selectedDrink = getDrinkDetails(drinkCode);
    // getDrinkDetails에서 오류 발생 시 currentState_가 HANDLING_ERROR로 변경됨
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (currentState_ == ControllerState::HANDLING_ERROR) return; // 오류 발생 시 더 이상 진행 안 함
        pendingDrinkSelection_ = selectedDrink; // 오류 없으면 pendingDrinkSelection_ 설정
    }


    auto availability = inventoryService_.checkDrinkAvailabilityAndPrice(drinkCode); // UC2 (S), UC3 (S)-1, PFR R1.3
    
    std::lock_guard<std::mutex> lock(mtx_); // 상태 변경 및 공유 변수 접근 보호
    if (availability.isAvailable) { // (S) UC3.2: 재고 있음
        userInterface_.displayMessage(
            pendingDrinkSelection_->getName() + " 선택됨. 가격: " + std::to_string(availability.price) +
            "원. (재고: " + std::to_string(availability.currentStock) + "개)"
        );
        currentActiveOrder_ = orderService_.createOrder(myVendingMachineId_, *pendingDrinkSelection_);
        isCurrentOrderPrepayment_ = false;
        currentState_ = ControllerState::AWAITING_PAYMENT_CONFIRMATION; // UC4로
    } else { // (S) UC3.3: 재고 없음
        if (pendingDrinkSelection_ && availability.price > 0) { // 음료는 존재하나 재고만 없는 경우
             userInterface_.displayOutOfStockMessage(pendingDrinkSelection_->getName());
        }
        currentState_ = ControllerState::BROADCASTING_STOCK_REQUEST; // UC8로
    }
}

// UC4: 사용자 결제 요청 / UC11: 선결제 결정 후 결제 요청
void UserProcessController::state_awaitingPaymentConfirmation() {
    // 이 함수는 사용자 입력을 받으므로, 뮤텍스는 입력 전후로 최소화.
    domain::Drink drinkToPay; // 복사본 사용
    bool isPrepay;
    std::optional<domain::VendingMachine> targetVm;
    int price;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!currentActiveOrder_ || !pendingDrinkSelection_) {
            last_error_info_ = errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "결제 확인 단계: 정보 누락");
            currentState_ = ControllerState::HANDLING_ERROR;
            cv_data_ready_ = true; cv_.notify_one(); // 메인 루프에 알림
            return;
        }
        drinkToPay = *pendingDrinkSelection_;
        isPrepay = isCurrentOrderPrepayment_;
        targetVm = selectedTargetVmForPrepayment_;
        price = drinkToPay.getPrice();
    }

    std::string action_type = isPrepay ? "선결제" : "구매";
    std::string target_info = "";
    if(isPrepay && targetVm){
        target_info = " (대상 자판기: " + targetVm->getId() + ")";
    }
    userInterface_.displayMessage(drinkToPay.getName() + " " + action_type + target_info + ". 가격: " + std::to_string(price) + "원.");
    userInterface_.displayPaymentPrompt(price); // (S) UC4.1 / UC11.1

    if (userInterface_.confirmPayment()) { // (A) UC4.2 / UC11.2 (블로킹 입력)
        changeState(ControllerState::PROCESSING_PAYMENT);
    } else {
        userInterface_.displayMessage(action_type + "가 취소되었습니다.");
        changeState(ControllerState::DISPLAYING_MAIN_MENU);
    }
}

// UC4, UC5, UC6: 실제 결제 시도 및 결과 처리
void UserProcessController::state_processingPayment() {
    domain::Order orderToProcess;
    bool isPrepay;
    { // 공유 자원 접근 최소화
        std::lock_guard<std::mutex> lock(mtx_);
        if (!currentActiveOrder_ || !pendingDrinkSelection_) {
            last_error_info_ = errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "결제 처리 단계: 정보 누락");
            currentState_ = ControllerState::HANDLING_ERROR;
            cv_data_ready_ = true; cv_.notify_one();
            return;
        }
        orderToProcess = *currentActiveOrder_; // 복사본 사용
        isPrepay = isCurrentOrderPrepayment_;
    }

    userInterface_.displayPaymentProcessing();
    network::PaymentCallbackReceiver paymentSim;
    bool paymentSuccess = false;
    paymentSim.simulatePrepayment([&paymentSuccess](bool success) { paymentSuccess = success; }, 3); // (S) UC4.3

    std::lock_guard<std::mutex> lock(mtx_); // 결과 처리 및 상태 변경 보호
    if (paymentSuccess) { // (S) UC5.1
        userInterface_.displayPaymentResult(true, "결제가 성공적으로 완료되었습니다!");
        
        orderService_.processOrderApproval(*currentActiveOrder_, isPrepay); // (S) UC5.2
        // // <<< 디버그 로그 추가 >>>
        // std::cout << "[" << myVendingMachineId_ << "] DEBUG: state_processingPayment - Payment success. isPrepay: " << isPrepay << std::endl;
        // if (selectedTargetVmForPrepayment_) {
        //     std::cout << "[" << myVendingMachineId_ << "] DEBUG: selectedTargetVmForPrepayment ID: " << selectedTargetVmForPrepayment_->getId() << std::endl;
        // } else {
        //     std::cout << "[" << myVendingMachineId_ << "] DEBUG: selectedTargetVmForPrepayment_ is NOT set." << std::endl;
        // }

        if (isPrepay) { // (S) UC5.2
            if (!selectedTargetVmForPrepayment_) {
                // std::cout << "[" << myVendingMachineId_ << "] ERROR: 선결제 대상 자판기 미선택 상태로 UC16 진입 시도!" << std::endl; 
                last_error_info_ = errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "선결제 대상 자판기 미선택");
                currentState_ = ControllerState::HANDLING_ERROR;
            } else {
                // std::cout << "[" << myVendingMachineId_ << "] DEBUG: Changing state to ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION" << std::endl; // <<< 로그 추가
                currentState_ = ControllerState::ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION; // UC16으로
            }
        } else { // (S) UC5.3
            currentState_ = ControllerState::DISPENSING_DRINK; // UC7로
        }
    } else { // (S) UC6.1
        userInterface_.displayPaymentResult(false, "결제에 실패했습니다.");
        orderService_.processOrderDeclination(*currentActiveOrder_); // (S) UC6.2
        currentState_ = ControllerState::DISPLAYING_MAIN_MENU; // (S) UC6.4
    }
}

// UC7, UC14: 음료 배출
void UserProcessController::state_dispensingDrink() {
    std::string drinkName;
    std::string certCodeForUsed; // 선결제 시 사용될 인증 코드
    bool prepaymentOrder = false;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!pendingDrinkSelection_ || !currentActiveOrder_) {
            last_error_info_ = errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "음료 배출 단계: 정보 누락");
            currentState_ = ControllerState::HANDLING_ERROR;
            cv_data_ready_ = true; cv_.notify_one();
            return;
        }
        drinkName = pendingDrinkSelection_->getName();
        prepaymentOrder = isCurrentOrderPrepayment_;
        if(prepaymentOrder) {
            certCodeForUsed = currentActiveOrder_->getCertCode();
        }
    }

    userInterface_.displayDispensingDrink(drinkName); // (S) UC7.1
    std::this_thread::sleep_for(std::chrono::seconds(2)); // 배출 시뮬레이션
    userInterface_.displayDrinkDispensed(drinkName);    // (S) UC7.3

    if (prepaymentOrder) { // 선결제 음료 수령 (UC14의 일부)
        prepaymentService_.changeAuthCodeStatusToUsed(certCodeForUsed); // (S) UC14.2
    }
    // 일반 구매 시 재고 차감(UC7.2)은 OrderService::processOrderApproval에서 이미 처리됨.
    changeState(ControllerState::TRANSACTION_COMPLETED_RETURN_TO_MENU);
}



void UserProcessController::state_broadcastingStockRequest() {
    std::string drinkCodeToBroadcast;
    std::string drinkNameToBroadcast; // 사용자 안내 메시지용

    { // pendingDrinkSelection_ 접근을 위한 잠금 범위
        std::lock_guard<std::mutex> lock(mtx_);
        if (!pendingDrinkSelection_) {
            // 예상치 못한 상황: 브로드캐스트를 시작하려 했으나 선택된 음료 정보가 없음
            last_error_info_ = errorService_.processOccurredError(
                ErrorType::UNEXPECTED_SYSTEM_ERROR,
                "재고 조회 브로드캐스트 시작 실패: 선택된 음료 정보가 없습니다.");
            currentState_ = ControllerState::HANDLING_ERROR;
            return;
        }
        drinkCodeToBroadcast = pendingDrinkSelection_->getDrinkCode();
        drinkNameToBroadcast = pendingDrinkSelection_->getName();
    } // 뮤텍스 해제

    userInterface_.displayMessage(drinkNameToBroadcast + " 음료의 재고를 주변 자판기에 문의합니다...");

    try {
        // 새로운 응답을 기다리기 위해 이전 결과 초기화 및 플래그 설정
        {
            std::lock_guard<std::mutex> lock(mtx_); // 공유 데이터(availableOtherVmsForDrink_ 등) 보호
            availableOtherVmsForDrink_.clear(); // 이전 다른 자판기 목록 초기화
            cv_data_ready_ = false;             // 응답 대기를 위한 플래그 리셋
            current_timeout_duration_ = std::chrono::seconds(3); // UC9 E2: 3초 이내 응답 없을 시 타임아웃 
        }

        // 실제 브로드캐스트 요청
        // MessageService는 생성 시 자신의 ID(myVmId_)를 알고 있으므로, broadcastStockRequest에는 drinkCode만 전달합니다.
        // MessageService의 sendStockRequestBroadcast 내부에서 dst_id = "0" (브로드캐스트)으로 설정됩니다. ]
        messageService_.sendStockRequestBroadcast(drinkCodeToBroadcast); // 

        // 응답 대기 타이머 시작
        startResponseTimer(current_timeout_duration_, ControllerState::AWAITING_STOCK_RESPONSES);

        // 응답 대기 상태로 전환
        changeState(ControllerState::AWAITING_STOCK_RESPONSES);

    } catch (const std::exception& e) {
        // messageService_에서 발생한 예외 처리 (예: 네트워크 오류 - UC8 E1
        std::lock_guard<std::mutex> lock(mtx_); // last_error_info_ 및 currentState_ 보호
        last_error_info_ = errorService_.processOccurredError(
            ErrorType::NETWORK_COMMUNICATION_ERROR, // 또는 MESSAGE_SEND_FAILED 등 ErrorService에 정의된 타입
            "주변 자판기 재고 조회 요청 전송 실패: " + std::string(e.what())
        );
        currentState_ = ControllerState::HANDLING_ERROR;
    }
}

// UC10, UC11: 다른 자판기 옵션 표시 및 선결제 결정
void UserProcessController::state_displayingOtherVmOptions() {
    // 이 함수는 AWAITING_STOCK_RESPONSES에서 타임아웃 또는 모든 응답 수신 후 호출됨
    std::optional<domain::Drink> currentDrinkSelection;
    std::vector<service::OtherVendingMachineInfo> currentAvailableVms;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!pendingDrinkSelection_) {
            last_error_info_ = errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "다른 자판기 옵션: 음료 정보 없음");
            currentState_ = ControllerState::HANDLING_ERROR;
            cv_data_ready_ = true; cv_.notify_one();
            return;
        }
        currentDrinkSelection = pendingDrinkSelection_;
        currentAvailableVms = availableOtherVmsForDrink_; // 복사
    }

    if (currentAvailableVms.empty()) {
        userInterface_.displayNoOtherVendingMachineFound(currentDrinkSelection->getName());
        changeState(ControllerState::DISPLAYING_MAIN_MENU);
        return;
    }

    try { // UC10 (S)-1
        std::optional<domain::VendingMachine> nearestVm = distanceService_.findNearestAvailableVendingMachine(
            myVendingMachineX_, myVendingMachineY_, currentAvailableVms
        );
        if (nearestVm) {
            { // selectedTargetVmForPrepayment_ 업데이트 보호
                std::lock_guard<std::mutex> lock(mtx_);
                selectedTargetVmForPrepayment_ = nearestVm;
            }
            userInterface_.displayNearestVendingMachine(*nearestVm, currentDrinkSelection->getName());

            if (userInterface_.confirmPrepayment(currentDrinkSelection->getName())) { // UC11 (S)-1, (A)-2 (블로킹 입력)
                std::lock_guard<std::mutex> lock(mtx_); // 공유 변수 업데이트 보호
                currentActiveOrder_ = orderService_.createOrder(myVendingMachineId_, *currentDrinkSelection);
                isCurrentOrderPrepayment_ = true;
                currentState_ = ControllerState::AWAITING_PAYMENT_CONFIRMATION; // UC11 (S)-3 -> UC4
            } else {
                userInterface_.displayMessage("선결제가 취소되었습니다.");
                changeState(ControllerState::DISPLAYING_MAIN_MENU);
            }
        } else {
            userInterface_.displayNoOtherVendingMachineFound(currentDrinkSelection->getName());
            state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "구매 가능한 가까운 자판기 선택 실패."));
        }
    } catch (const std::exception& e) {
        userInterface_.displayNoOtherVendingMachineFound(currentDrinkSelection->getName());
        state_handlingError(errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "가까운 자판기 처리 중 예외: " + std::string(e.what())));
    }
}

// UC16: 재고 확보 요청 전송
void UserProcessController::state_issuingAuthCodeAndRequestingReservation() {
    std::string targetVmId, drinkCode, certCode, drinkName;
    { // 공유 변수 읽기 보호
      // <<< 각 정보 유효성 검사 로그 추가 >>>
        std::lock_guard<std::mutex> lock(mtx_);
        // if (!currentActiveOrder_) std::cout << "[" << myVendingMachineId_ << "] DEBUG: currentActiveOrder_ 없음" << std::endl;
        // else if (currentActiveOrder_->getCertCode().empty())  std::cout << "[" << myVendingMachineId_ << "] DEBUG: currentActiveOrder_에 certCode 없음" << std::endl;
        // if (!pendingDrinkSelection_) std::cout << "[" << myVendingMachineId_ << "] DEBUG: pendingDrinkSelection_ 없음" << std::endl;
        // if (!selectedTargetVmForPrepayment_) std::cout << "[" << myVendingMachineId_ << "] DEBUG: selectedTargetVmForPrepayment_ 없음" << std::endl;
        // if (!isCurrentOrderPrepayment_) std::cout << "[" << myVendingMachineId_ << "] DEBUG: isCurrentOrderPrepayment_가 false임" << std::endl;

        if (!currentActiveOrder_ || currentActiveOrder_->getCertCode().empty() || !pendingDrinkSelection_ || !selectedTargetVmForPrepayment_ || !isCurrentOrderPrepayment_) {
            last_error_info_ = errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "선결제 요청: 정보 부족");
            currentState_ = ControllerState::HANDLING_ERROR;
            cv_data_ready_ = true; cv_.notify_one();
            return;
        }
        targetVmId = selectedTargetVmForPrepayment_->getId();
        drinkCode = currentActiveOrder_->getDrinkCode();
        certCode = currentActiveOrder_->getCertCode();
        drinkName = pendingDrinkSelection_->getName();
        cv_data_ready_ = false; // 새로운 응답 대기 시작
        current_timeout_duration_ = std::chrono::seconds(10); // 타임아웃 설정 (예: 30초)
    }

    userInterface_.displayMessage(targetVmId + "에 " + drinkName + " 재고 확보 요청 (인증코드: " + certCode + ")");
    messageService_.sendPrepaymentReservationRequest(targetVmId, drinkCode, certCode); // UC16 (S)-1
    startResponseTimer(current_timeout_duration_, ControllerState::ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION);
}

// UC12: 인증코드 발급 (안내)
void UserProcessController::state_displayingAuthCodeInfo() {
    // 이 함수는 onRespPrepayReceived 콜백에서 상태 변경 후 호출됨
    domain::Order orderToShow;
    domain::VendingMachine targetVmToShow;
    std::string drinkNameToShow;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!currentActiveOrder_ || currentActiveOrder_->getCertCode().empty() || !selectedTargetVmForPrepayment_ || !pendingDrinkSelection_) {
            last_error_info_ = errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드 안내: 정보 부족");
            currentState_ = ControllerState::HANDLING_ERROR;
            cv_data_ready_ = true; cv_.notify_one(); // 메인 루프에 알림
            return;
        }
        orderToShow = *currentActiveOrder_;
        targetVmToShow = *selectedTargetVmForPrepayment_;
        drinkNameToShow = pendingDrinkSelection_->getName();
    }

    userInterface_.displayAuthCode(orderToShow.getCertCode(),targetVmToShow,drinkNameToShow); // UC12 (S)-2, (S)-3
    changeState(ControllerState::TRANSACTION_COMPLETED_RETURN_TO_MENU);
}

// UC13, UC14: 인증코드 입력 및 유효성 검증
void UserProcessController::state_awaitingAuthCodeInputPrompt() {
    std::string authCode = userInterface_.getAuthCodeInput(); // UC13 (A)-1, (A)-2 (블로킹 입력)
    if (authCode.empty()) {
        userInterface_.displayMessage("인증 코드 입력이 취소되었습니다.");
        changeState(ControllerState::DISPLAYING_MAIN_MENU);
        return;
    }

    std::lock_guard<std::mutex> lock(mtx_); // 이후 공유 변수 접근 보호
    if (!prepaymentService_.isValidAuthCodeFormat(authCode)) { // UC13 (S)-3
        last_error_info_ = errorService_.processOccurredError(ErrorType::AUTH_CODE_INVALID_FORMAT, "입력 코드: " + authCode);
        currentState_ = ControllerState::HANDLING_ERROR; // UC13 E1
        return;
    }

    domain::PrePaymentCode prepayDetails = prepaymentService_.getPrepaymentDetailsIfActive(authCode); // UC14 (S)-1
    if (prepayDetails.getCode().empty()) { // AUTH_CODE_NOT_FOUND 또는 AUTH_CODE_ALREADY_USED (UC14 E1)
        // PrepaymentService 내부에서 ErrorService가 호출되었을 것이므로,
        // 이미 last_error_info_가 설정되어 있을 것.
        last_error_info_ = errorService_.processOccurredError(ErrorType::AUTH_CODE_NOT_FOUND, "입력 코드 " + authCode + "는 유효하지 않거나 이미 사용되었습니다.");
        currentState_ = ControllerState::HANDLING_ERROR;
        return;
    }

    std::shared_ptr<domain::Order> heldOrderPtr = prepayDetails.getHeldOrder();
    if (!heldOrderPtr) {
        last_error_info_ = errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드에 연결된 주문 객체 없음");
        currentState_ = ControllerState::HANDLING_ERROR;
        return;
    }
    currentActiveOrder_ = *heldOrderPtr;

    domain::Drink tempDrink = getDrinkDetails(currentActiveOrder_->getDrinkCode()); // 임시 변수에 받아 상태 변경 확인
    if (currentState_ == ControllerState::HANDLING_ERROR) return; // getDrinkDetails에서 오류 발생
    pendingDrinkSelection_ = tempDrink;


    if (!pendingDrinkSelection_ || pendingDrinkSelection_->getDrinkCode().empty()) { // 위의 getDrinkDetails에서 오류 처리했어야 함
        last_error_info_ = errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드의 음료 정보 조회 실패");
        currentState_ = ControllerState::HANDLING_ERROR;
        return;
    }
    
    isCurrentOrderPrepayment_ = true;
    userInterface_.displayMessage("인증 코드 (" + authCode + ") 확인 완료: " + pendingDrinkSelection_->getName());
    currentState_ = ControllerState::DISPENSING_DRINK; // UC14 (S)-3 -> UC7
}

void UserProcessController::state_transactionCompletedReturnToMenu() {
    userInterface_.displayMessage("거래가 완료되었습니다. 감사합니다.");
    resetCurrentTransactionState(); // 내부에서 뮤텍스 사용
    changeState(ControllerState::DISPLAYING_MAIN_MENU); // 내부에서 뮤텍스 사용
}

void UserProcessController::state_handlingError(const service::ErrorInfo& errorInfo) {
    userInterface_.displayError(errorInfo.userFriendlyMessage);
    ControllerState nextState;
    switch (errorInfo.resolutionLevel) {
        case ErrorResolutionLevel::RETRY_INPUT:
            userInterface_.displayMessage("입력을 다시 시도해주세요. (메인 메뉴로 돌아갑니다)");
            nextState = ControllerState::DISPLAYING_MAIN_MENU; 
            break;
        case ErrorResolutionLevel::SYSTEM_FATAL_ERROR:
            userInterface_.displayMessage("치명적인 시스템 오류가 발생하여 시스템을 종료합니다.");
            nextState = ControllerState::SYSTEM_HALTED_REQUEST;
            break;
        case ErrorResolutionLevel::RETURN_TO_MAIN_MENU:
            userInterface_.displayMessage("메인 메뉴로 돌아갑니다.");
            nextState = ControllerState::DISPLAYING_MAIN_MENU;
            break;
        default:
            userInterface_.displayMessage("초기 화면으로 돌아갑니다.");
            nextState = ControllerState::DISPLAYING_MAIN_MENU;
            break;
    }
    changeState(nextState); // 상태 변경
}

// --- 콜백 핸들러들 (io_context 스레드에서 실행) ---

void UserProcessController::onReqStockReceived(const network::Message& msg) { // UC17
    std::lock_guard<std::mutex> lock(mtx_); // 공유 자원 접근 보호
    if (msg.msg_content.count("item_code")) { // (S) UC17.1
        std::string requestedDrinkCode = msg.msg_content.at("item_code");
        auto availabilityInfo = inventoryService_.checkDrinkAvailabilityAndPrice(requestedDrinkCode); // (S) UC17.2
        messageService_.sendStockResponse(msg.src_id, requestedDrinkCode, availabilityInfo.currentStock); // (S) UC17.3
    } else { // UC17 E1
        last_error_info_ = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "REQ_STOCK (item_code 누락 from " + msg.src_id + ")");
        currentState_ = ControllerState::HANDLING_ERROR; // 메인 스레드가 처리하도록 상태 변경
        cv_data_ready_ = true; 
        cv_.notify_one();
    }
}

void UserProcessController::onRespStockReceived(const network::Message& msg) { // UC9
    std::lock_guard<std::mutex> lock(mtx_);
    // // <<< 디버그 로그 추가 >>>
    // std::cout << "[" << myVendingMachineId_ << "] onRespStockReceived: 메시지 수신됨 (from: " << msg.src_id << ", type: " << static_cast<int>(msg.msg_type) << ")" << std::endl;
    // if (msg.msg_content.count("item_code")) {
    //     std::cout << "  - item_code: " << msg.msg_content.at("item_code") << std::endl;
    // }

    if (currentState_ != ControllerState::AWAITING_STOCK_RESPONSES) {
    //     // <<< 디버그 로그 추가 >>>
    //     std::cout << "  - 현재 상태 AWAITING_STOCK_RESPONSES 아님. 메시지 무시." << std::endl;
        return;
    }
    if (currentState_ != ControllerState::AWAITING_STOCK_RESPONSES) return;

    try {
        std::string drinkCode = msg.msg_content.at("item_code");
        int stockQty = std::stoi(msg.msg_content.at("item_num"));
        int x = std::stoi(msg.msg_content.at("coor_x"));
        int y = std::stoi(msg.msg_content.at("coor_y"));
        std::string vmId = msg.src_id;

        if (pendingDrinkSelection_ && pendingDrinkSelection_->getDrinkCode() == drinkCode && stockQty > 0) { // (A) UC9.1
            bool alreadyReceived = false;
            for(const auto& ovm : availableOtherVmsForDrink_) if(ovm.id == vmId) alreadyReceived = true;
            if(alreadyReceived) return;

            availableOtherVmsForDrink_.push_back({vmId, x, y, true});
            // // <<< 디버그 로그 추가 >>>
            // std::cout << "  - 유효한 재고 응답. availableOtherVmsForDrink_ 크기: " << availableOtherVmsForDrink_.size() << "/" << total_other_vms_ << std::endl;

            // // <<< 디버그 로그 추가 >>>
            // std::cout << "  - 모든 다른 VM으로부터 응답 수신 완료 또는 지정된 수만큼 받음. 타이머 취소 및 상태 변경 시도." << std::endl;
            
        
            if (availableOtherVmsForDrink_.size() >= total_other_vms_) {
                response_timer_.cancel(); // 모든 응답 수신, 타이머 취소
                currentState_ = ControllerState::DISPLAYING_OTHER_VM_OPTIONS; // (S) UC9.2 -> UC10
                cv_data_ready_ = true;
                cv_.notify_one();
            }else{
                
            //  // <<< 디버그 로그 추가 >>>
            //  std::cout << "  - 유효하지 않은 재고 응답 또는 조건 불일치 (요청 음료: "
            //    << (pendingDrinkSelection_ ? pendingDrinkSelection_->getDrinkCode() : "N/A")
            //       << ", 수신 음료: " << drinkCode << ", 재고: " << stockQty << ")" << std::endl;
    
            }
        }
    } catch (const std::exception& e) { // UC9 E1
        last_error_info_ = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "RESP_STOCK 처리 오류 (from " + msg.src_id + "): " + e.what());
        currentState_ = ControllerState::HANDLING_ERROR;
        cv_data_ready_ = true;
        cv_.notify_one();
    }
}

void UserProcessController::onReqPrepayReceived(const network::Message& msg) { // UC15
    // std::cout << "[" << myVendingMachineId_ << "] DEBUG: onReqPrepayReceived 진입 (from: " << msg.src_id << ")" << std::endl; // <<< 로그 추가
    std::lock_guard<std::mutex> lock(mtx_);
    try { // (S) UC15.1
        std::string drinkCode = msg.msg_content.at("item_code");
        std::string certCode = msg.msg_content.at("cert_code");
        std::string requestingVmId = msg.src_id;
        int requestedItemNum = 1;
        if (msg.msg_content.count("item_num")) {
            try {
                requestedItemNum = std::stoi(msg.msg_content.at("item_num"));
                if (requestedItemNum <= 0 || requestedItemNum > 99) {
                    last_error_info_ = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "REQ_PREPAY (잘못된 item_num from " + msg.src_id + ")");
                    currentState_ = ControllerState::HANDLING_ERROR; // 또는 직접 실패 응답 전송
                    cv_data_ready_ = true; cv_.notify_one();
                    messageService_.sendPrepaymentReservationResponse(requestingVmId, drinkCode, 0, false);
                    return;
                }
            } catch (const std::exception&) {
                last_error_info_ = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "REQ_PREPAY (item_num 파싱 오류 from " + msg.src_id + ")");
                currentState_ = ControllerState::HANDLING_ERROR;
                cv_data_ready_ = true; cv_.notify_one();
                messageService_.sendPrepaymentReservationResponse(requestingVmId, drinkCode, 0, false);
                return;
            }
        }
        bool reservationSuccess = false;
        auto availabilityInfo = inventoryService_.checkDrinkAvailabilityAndPrice(drinkCode);
        if (availabilityInfo.isAvailable && availabilityInfo.currentStock >= requestedItemNum) { // (S) UC15.2
            inventoryService_.decreaseStockByAmount(drinkCode, requestedItemNum);
            prepaymentService_.recordIncomingPrepayment(certCode, drinkCode, requestingVmId);
            reservationSuccess = true;
        } else { /* (A1) UC15 */ }
        messageService_.sendPrepaymentReservationResponse(requestingVmId, drinkCode, (reservationSuccess ? requestedItemNum : 0), reservationSuccess); // (S) UC15.3
    } catch (const std::exception& e) {
        last_error_info_ = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "REQ_PREPAY 처리 오류 (from " + msg.src_id + "): " + e.what());
        currentState_ = ControllerState::HANDLING_ERROR;
        cv_data_ready_ = true; cv_.notify_one();
    }
}

void UserProcessController::onRespPrepayReceived(const network::Message& msg) { // UC16
    std::lock_guard<std::mutex> lock(mtx_);
    if (currentState_ != ControllerState::ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION) return;
    response_timer_.cancel(); // 응답 수신, 타이머 취소

    try {
        std::string receivedDrinkCode = msg.msg_content.at("item_code");
        std::string availability = msg.msg_content.at("availability");

        if (pendingDrinkSelection_ && pendingDrinkSelection_->getDrinkCode() == receivedDrinkCode &&
            currentActiveOrder_ && !currentActiveOrder_->getCertCode().empty() &&
            selectedTargetVmForPrepayment_ && selectedTargetVmForPrepayment_->getId() == msg.src_id) {
            if (availability == "T") { // (S) UC16.2
                currentState_ = ControllerState::DISPLAYING_AUTH_CODE_INFO; // UC12로
            } else { // (E1) UC16
                last_error_info_ = errorService_.processOccurredError(ErrorType::STOCK_RESERVATION_FAILED_AT_OTHER_VM, msg.src_id);
                currentState_ = ControllerState::HANDLING_ERROR;
            }
        } else {
            last_error_info_ = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "RESP_PREPAY (내용 불일치 from " + msg.src_id + ")");
            currentState_ = ControllerState::HANDLING_ERROR;
        }
    } catch (const std::exception& e) {
        last_error_info_ = errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "RESP_PREPAY 처리 오류 (from " + msg.src_id + "): " + e.what());
        currentState_ = ControllerState::HANDLING_ERROR;
    }
    cv_data_ready_ = true; // 메인 스레드 깨우기
    cv_.notify_one();
}

} // namespace service
