#pragma once

#include <string>
#include <vector>
#include <memory>   // 참조를 사용하지만, 의존성 주입 명시를 위해 포함 (습관적)
#include <optional> // currentActiveOrder_ 등에 사용
#include <chrono>   // std::chrono 사용

// --- 필요한 타입들의 전체 정의를 먼저 포함 ---
#include "domain/drink.h"          // 또는 .hpp
#include "domain/order.h"          // 또는 .hpp
#include "domain/vendingMachine.h" // 또는 .hpp
#include "network/message.hpp"     // network::Message 정의

// --- 서비스 및 UI 클래스들은 전방 선언 가능 (참조 또는 포인터로만 사용 시) ---
// 하지만 생성자에서 참조로 받고 멤버로 저장하므로, 실제로는 .cpp에서 include 하거나,
// 여기서도 include 하는 것이 더 안전할 수 있음 (특히 Presentation/UserInterface)
namespace presentation { class UserInterface; }
namespace service {
    class InventoryService;
    class OrderService;
    class PrepaymentService;
    class MessageService;
    class DistanceService;
    class ErrorService;
    struct ErrorInfo;
    struct OtherVendingMachineInfo; // from DistanceService.hpp
}
// network::Message 는 위에서 이미 include 함.

namespace boost { namespace asio { class io_context; }}


namespace service {

// UserProcessController의 내부 상태
enum class ControllerState {
    // 초기화 및 공통 상태
    INITIALIZING,
    SYSTEM_READY,
    DISPLAYING_MAIN_MENU,       // UC1: 주 음료 메뉴 표시 (음료 목록 + 인증 코드 입력 옵션)
    AWAITING_MAIN_MENU_CHOICE,  // 주 메뉴 선택 대기 (음료 선택 또는 인증 코드 입력)
    SYSTEM_HALTED_REQUEST, // 시스템 정지 요청 (사용자 또는 내부 오류로 인한)

    // 음료 직접 구매 흐름
    AWAITING_DRINK_SELECTION,   // 음료 코드 입력 대기 (메뉴에서 음료 선택 시)
    PROCESSING_DRINK_SELECTION, // UC2, UC3: 선택된 음료 로컬 재고 확인 중
    AWAITING_PAYMENT_CONFIRMATION, // UC4: (재고 있을 시, 또는 선결제 시) 결제 진행 여부 확인 (상태 통합)
    PROCESSING_PAYMENT,         // UC4: 결제 모듈과 통신 중 (카드사 응답 대기) (상태 통합)
    DISPENSING_DRINK,           // UC7: 음료 배출 중

    // 다른 자판기 조회 및 선결제 흐름
    BROADCASTING_STOCK_REQUEST, // UC8: 주변 자판기에 재고 문의 중
    AWAITING_STOCK_RESPONSES,   // UC9: 재고 문의 응답 대기 중 (타임아웃 처리 필요)
    DISPLAYING_OTHER_VM_OPTIONS,// UC10: 가장 가까운 자판기 정보 표시
    AWAITING_PREPAYMENT_CHOICE, // UC11: 사용자에게 선결제 여부 확인 (다른 자판기 음료에 대해)
    ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION, // UC12, UC16: 인증 코드 발급 및 다른 자판기에 재고 확보 요청 중 (이름 변경)
    DISPLAYING_AUTH_CODE_INFO,  // UC12: 사용자에게 인증 코드 및 수령 정보 안내

    // 인증 코드로 음료 받기 흐름
    AWAITING_AUTH_CODE_INPUT_PROMPT, // 주 메뉴에서 "인증 코드로 음료 받기" 선택 시 인증 코드 입력 안내 상태
    AWAITING_AUTH_CODE_INPUT,   // UC13: 사용자로부터 인증 코드 입력 대기
    VALIDATING_AUTH_CODE,       // UC14: 입력된 인증 코드 유효성 검증 중

    // 공통 마무리 또는 오류 상태
    TRANSACTION_COMPLETED_RETURN_TO_MENU, // 한 사이클의 거래 완료, 메인 메뉴로 돌아갈 준비
    HANDLING_ERROR              // 오류 발생 인지, ErrorInfo 바탕으로 다음 행동 결정 중
    // SYSTEM_HALTED // ErrorInfo의 ErrorResolutionLevel::SYSTEM_FATAL_ERROR로 처리
};


class UserProcessController {
public:
    UserProcessController(
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
    );
    // ~UserProcessController() = default; // 상속 고려 안 함

    void run(); // 자판기 시스템의 메인 실행 루프

private:
    presentation::UserInterface& userInterface_;
    service::InventoryService& inventoryService_;
    service::OrderService& orderService_;
    service::PrepaymentService& prepaymentService_;
    service::MessageService& messageService_;
    service::DistanceService& distanceService_;
    service::ErrorService& errorService_;
    boost::asio::io_context& ioContext_;

    std::string myVendingMachineId_;
    int myVendingMachineX_;
    int myVendingMachineY_;

    ControllerState currentState_;
    std::optional<domain::Order> currentActiveOrder_; // 현재 진행 중인 주문 정보
    bool isCurrentOrderPrepayment_;                   // 현재 주문이 선결제인지 여부 플래그
    
    // 임시 데이터 저장용 (예: 다른 자판기 조회 결과, 선택된 음료 등)
    std::optional<domain::Drink> pendingDrinkSelection_; // 사용자가 선택했지만 아직 주문 확정 전인 음료
    std::vector<service::OtherVendingMachineInfo> availableOtherVmsForDrink_; // 다른 자판기 재고 조회 결과
    std::optional<domain::VendingMachine> selectedTargetVmForPrepayment_; // 선결제 대상 자판기

    std::optional<std::chrono::steady_clock::time_point> timeout_start_time_; // 'UserProcessController::' 제거
    std::chrono::seconds current_timeout_duration_;                             // 'UserProcessController::' 제거
    int total_other_vms_ = 7; // 주변 자판기 수 (이 선언은 문제 없어 보입니다)


    // --- 상태별 처리 메소드 ---
    void processCurrentState(); // 현재 상태에 따라 적절한 핸들러 호출
    
    // --- 유스케이스 단계별 주요 핸들러 메소드 (private) ---
    void initializeSystemAndRegisterMessageHandlers();

    void state_displayingMainMenu();
    void state_awaitingMainMenuChoice();

    void state_awaitingDrinkSelection();
    void state_processingDrinkSelection(const std::string& drinkCode);

    void state_awaitingPaymentConfirmation();
    void state_processingPayment(); // 일반 구매 및 선결제 공통 사용
    
    void state_dispensingDrink();

    void state_broadcastingStockRequest();
    void state_awaitingStockResponses(); // 타임아웃 로직 포함
    void state_displayingOtherVmOptions();
    void state_awaitingPrepaymentChoice();

    void state_issuingAuthCodeAndRequestingReservation();
    void state_displayingAuthCodeInfo();

    void state_awaitingAuthCodeInputPrompt();
    void state_awaitingAuthCodeInput();
    void state_validatingAuthCode(const std::string& authCode);

    void state_transactionCompletedReturnToMenu();
    void state_handlingError(const service::ErrorInfo& errorInfo);


    // --- MessageService로부터 호출될 콜백 핸들러들 ---
    void onReqStockReceived(const network::Message& msg);
    void onRespStockReceived(const network::Message& msg);
    void onReqPrepayReceived(const network::Message& msg);
    void onRespPrepayReceived(const network::Message& msg);

    // --- 공통 유틸리티 ---
    void changeState(ControllerState newState);
    void resetCurrentTransactionState(); // 현재 주문 관련 임시 상태 초기화
    domain::Drink getDrinkDetails(const std::string& drinkCode); // InventoryService/DrinkRepository 통해 음료 상세 조회
};

} // namespace service