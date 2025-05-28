#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>   // std::chrono::steady_clock, std::chrono::seconds
#include <memory>   // std::shared_ptr 

#include "domain/drink.h"          // domain::Drink 객체 사용
#include "domain/order.h"          // domain::Order 객체 사용
#include "domain/vendingMachine.h" // domain::VendingMachine 객체 사용
#include "network/message.hpp"     // network::Message 구조체 사용
#include "service/DistanceService.hpp" // service::OtherVendingMachineInfo 구조체 사용
#include "service/ErrorService.hpp"    // service::ErrorInfo 구조체 사용

// --- 서비스 및 UI 클래스 (생성자에서 참조로 주입받음) ---
namespace presentation { class UserInterface; }
namespace service {
    class InventoryService;
    class OrderService;
    class PrepaymentService;
    class MessageService;
}

namespace boost { namespace asio { class io_context; }}

namespace service {

/**
 * @brief 자판기 시스템의 전체 사용자 상호작용 흐름과 비즈니스 로직을 제어하는 메인 컨트롤러.
 * 상태 머신으로 동작하며, 각 상태에 따라 적절한 서비스를 호출하고 UI와 상호작용합니다.
 */
enum class ControllerState {
    // 초기화 및 기본 상태
    INITIALIZING,                       // 시스템 초기화 중
    SYSTEM_READY,                       // 시스템 준비 완료, 메인 메뉴 표시 대기
    DISPLAYING_MAIN_MENU,               // UC1: 메인 메뉴 표시 및 사용자 선택 대기
    SYSTEM_HALTED_REQUEST,              // 시스템 종료 요청됨

    // 음료 직접 구매 흐름
    AWAITING_DRINK_SELECTION,           // UC2: 사용자 음료 선택 대기
    AWAITING_PAYMENT_CONFIRMATION,      // UC4, UC11: 결제 진행 여부 사용자 확인 대기
    PROCESSING_PAYMENT,                 // UC4, UC5, UC6: 결제 처리 중 (카드사 응답 대기)
    DISPENSING_DRINK,                   // UC7, UC14: 음료 배출 중

    // 다른 자판기 조회 및 선결제 흐름
    BROADCASTING_STOCK_REQUEST,         // UC8: 주변 자판기에 재고 문의 메시지 전송
    AWAITING_STOCK_RESPONSES,           // UC9: 재고 문의 응답 대기 (타임아웃 처리)
    DISPLAYING_OTHER_VM_OPTIONS,        // UC10: 가장 가까운 자판기 정보 표시, UC11: 선결제 여부 확인
    ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION, // UC16: 다른 자판기에 재고 확보 요청 및 응답 대기 (타임아웃 처리)
    DISPLAYING_AUTH_CODE_INFO,          // UC12: 사용자에게 인증 코드 및 수령 정보 안내

    // 인증 코드로 음료 받기 흐름
    AWAITING_AUTH_CODE_INPUT_PROMPT,    // UC13: 인증 코드 입력 안내 및 대기

    // 공통 마무리 또는 오류 상태
    TRANSACTION_COMPLETED_RETURN_TO_MENU, // 거래 완료, 메인 메뉴로 복귀 준비
    HANDLING_ERROR                      // 오류 발생 처리 중
};

class UserProcessController {
public:
    /**
     * @brief UserProcessController 생성자.
     * 필요한 모든 서비스 및 객체들을 의존성 주입받습니다.
     * @param ui 사용자 인터페이스 객체.
     * @param inventoryService 재고 관련 서비스 객체.
     * @param orderService 주문 관련 서비스 객체.
     * @param prepaymentService 선결제 관련 서비스 객체.
     * @param messageService 메시지 송수신 서비스 객체.
     * @param distanceService 거리 계산 서비스 객체.
     * @param errorService 오류 처리 서비스 객체.
     * @param ioContext Boost.Asio io_context 객체 (네트워크 및 비동기 작업용).
     * @param myVmId 현재 자판기의 고유 ID.
     * @param myVmX 현재 자판기의 X 좌표.
     * @param myVmY 현재 자판기의 Y 좌표.
     */
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

    /**
     * @brief 자판기 시스템의 메인 실행 루프를 시작합니다.
     * 초기화 후, 현재 상태에 따라 적절한 작업을 수행하고 사용자 입력을 처리합니다.
     */
    void run();

private:
    // --- 의존성 주입된 객체들 ---
    presentation::UserInterface& userInterface_;
    service::InventoryService& inventoryService_;
    service::OrderService& orderService_;
    service::PrepaymentService& prepaymentService_;
    service::MessageService& messageService_;
    service::DistanceService& distanceService_;
    service::ErrorService& errorService_;
    boost::asio::io_context& ioContext_; // 비동기 작업 및 네트워크 이벤트 처리

    // --- 현재 자판기 정보 ---
    std::string myVendingMachineId_; // 현재 자판기의 ID
    int myVendingMachineX_;          // 현재 자판기의 X 좌표
    int myVendingMachineY_;          // 현재 자판기의 Y 좌표

    // --- 현재 거래 상태 정보 ---
    ControllerState currentState_;                          // 현재 컨트롤러의 상태
    std::optional<domain::Order> currentActiveOrder_;       // 현재 진행 중인 주문 정보
    bool isCurrentOrderPrepayment_ = false;                 // 현재 주문이 선결제인지 여부
    std::optional<domain::Drink> pendingDrinkSelection_;    // 사용자가 선택했지만 아직 주문 확정 전인 음료
    std::vector<service::OtherVendingMachineInfo> availableOtherVmsForDrink_; // 다른 자판기 재고 조회 결과
    std::optional<domain::VendingMachine> selectedTargetVmForPrepayment_; // 선결제 대상 자판기 정보

    // --- 타임아웃 처리용 멤버 변수 ---
    std::optional<std::chrono::steady_clock::time_point> timeout_start_time_; // 타임아웃 시작 시간
    std::chrono::seconds current_timeout_duration_;                             // 현재 대기 상태의 타임아웃 기간
    int total_other_vms_; // 나를 제외한 다른 자판기의 총 수 (응답 카운트용)


    // --- 내부 상태 처리 및 유틸리티 메소드 ---
    void processCurrentState(); // 현재 상태에 따라 적절한 핸들러 호출
    void changeState(ControllerState newState); // 컨트롤러 상태 변경
    void resetCurrentTransactionState(); // 현재 거래 관련 임시 상태 초기화
    domain::Drink getDrinkDetails(const std::string& drinkCode); // 음료 코드로부터 상세 정보 조회

    // --- 초기화 ---
    void initializeSystemAndRegisterMessageHandlers(); // 시스템 시작 시 초기화 및 메시지 핸들러 등록

    // --- 각 유스케이스 단계별 상태 처리 메소드 ---
    void state_displayingMainMenu();            // UC1: 메인 메뉴 표시
    void state_awaitingDrinkSelection();        // UC2, UC3: 음료 선택 및 로컬 재고 확인
    void state_awaitingPaymentConfirmation();   // UC4, UC11: 결제 확인
    void state_processingPayment();             // UC4, UC5, UC6: 결제 처리
    void state_dispensingDrink();               // UC7, UC14: 음료 배출
    void state_broadcastingStockRequest();      // UC8: 재고 조회 브로드캐스트
    void state_displayingOtherVmOptions();      // UC10, UC11: 다른 자판기 옵션 표시 및 선결제 결정
    void state_issuingAuthCodeAndRequestingReservation(); // UC16: 재고 확보 요청 및 응답 대기
    void state_displayingAuthCodeInfo();        // UC12: 인증 코드 안내
    void state_awaitingAuthCodeInputPrompt();   // UC13, UC14: 인증 코드 입력 및 검증
    void state_transactionCompletedReturnToMenu(); // 완료 메소드
    void state_handlingError(const service::ErrorInfo& errorInfo); // 오류 처리

    // --- MessageService로부터 호출될 콜백 핸들러들 ---
    void onReqStockReceived(const network::Message& msg);   // UC17: 외부 재고 조회 요청 수신
    void onRespStockReceived(const network::Message& msg);  // UC9:  재고 조회 응답 수신
    void onReqPrepayReceived(const network::Message& msg);  // UC15: 외부 선결제 요청 수신
    void onRespPrepayReceived(const network::Message& msg); // UC16: 선결제 재고 확보 응답 수신
};

} // namespace service
