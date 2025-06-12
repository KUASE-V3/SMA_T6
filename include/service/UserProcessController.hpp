#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <mutex>  // std::mutex, std::lock_guard, std::unique_lock
#include <memory> // std::shared_ptr (PrePaymentCode가 Order를 가짐)
#include <condition_variable> // std::condition_variable

#include "domain/drink.h"
#include "domain/order.h"
#include "domain/vendingMachine.h"
#include "domain/prepaymentCode.h" // state_awaitingAuthCodeInputPrompt에서 사용
#include "network/message.hpp"
#include "service/DistanceService.hpp" // service::OtherVendingMachineInfo
#include "service/ErrorService.hpp"    // service::ErrorInfo

#include "boost/asio/io_context.hpp" 
#include "boost/asio/steady_timer.hpp" // Asio 타이머

namespace presentation { class UserInterface; }
namespace service {
    class InventoryService;
    class OrderService;
    class PrepaymentService;
    class MessageService;
    // DistanceService와 ErrorService는 위에서 이미 include 함
}

namespace service {
/**
 * @brief 자판기 시스템의 전체 사용자 상호작용 흐름과 비즈니스 로직을 제어하는 메인 컨트롤러입니다.
 * 멀티스레드 환경에 대응하여, 메인 스레드는 UI 및 상태 전이를, 별도 io_context 스레드는
 * 네트워크 I/O 및 비동기 타이머를 처리합니다. 공유 데이터는 뮤텍스로 보호됩니다.
 * 관련된 유스케이스: UC1 ~ UC17
 */
enum class ControllerState {
    // 초기화 및 기본 상태
    INITIALIZING,                       ///< 시스템 초기화 중
    SYSTEM_READY,                       ///< 시스템 준비 완료, 메인 메뉴 표시 대기
    DISPLAYING_MAIN_MENU,               ///< UC1: 메인 메뉴 표시 및 사용자 선택 대기
    SYSTEM_HALTED_REQUEST,              ///< 시스템 종료 요청됨

    // 음료 직접 구매 흐름
    AWAITING_DRINK_SELECTION,           ///< UC2: 사용자 음료 선택 대기 (UC3으로 이어짐)
    AWAITING_PAYMENT_CONFIRMATION,      ///< UC4, UC11: 결제 진행 여부 사용자 확인 대기
    PROCESSING_PAYMENT,                 ///< UC4, UC5, UC6: 결제 처리 중
    DISPENSING_DRINK,                   ///< UC7 (일반구매), UC14 (선결제수령): 음료 배출 중

    // 다른 자판기 조회 및 선결제 흐름
    BROADCASTING_STOCK_REQUEST,         ///< UC8: 주변 자판기에 재고 문의 메시지 전송
    AWAITING_STOCK_RESPONSES,           ///< UC9: 재고 문의 응답 대기 (타임아웃 처리)
    DISPLAYING_OTHER_VM_OPTIONS,        ///< UC10: 가장 가까운 자판기 정보 표시, UC11: 선결제 여부 확인
    ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION, ///< UC16: 다른 자판기에 재고 확보 요청 및 응답 대기
    DISPLAYING_AUTH_CODE_INFO,          ///< UC12: 사용자에게 인증 코드 및 수령 정보 안내

    // 인증 코드로 음료 받기 흐름
    AWAITING_AUTH_CODE_INPUT_PROMPT,    ///< UC13, UC14: 인증 코드 입력 안내 및 검증 처리

    // 공통 마무리 또는 오류 상태
    TRANSACTION_COMPLETED_RETURN_TO_MENU, ///< 거래 완료, 메인 메뉴로 복귀 준비
    HANDLING_ERROR                      ///< 오류 발생 처리 중
};

class UserProcessController {
public:
    /**
     * @brief UserProcessController 생성자.
     * @param ui 사용자 인터페이스 객체에 대한 참조.
     * @param inventoryService 재고 관련 서비스 객체에 대한 참조.
     * @param orderService 주문 관련 서비스 객체에 대한 참조.
     * @param prepaymentService 선결제 관련 서비스 객체에 대한 참조.
     * @param messageService 메시지 송수신 서비스 객체에 대한 참조.
     * @param distanceService 거리 계산 서비스 객체에 대한 참조.
     * @param errorService 오류 처리 서비스 객체에 대한 참조.
     * @param ioContext Boost.Asio io_context 객체에 대한 참조 (네트워크 및 비동기 작업용).
     * @param myVmId 현재 자판기의 고유 ID.
     * @param myVmX 현재 자판기의 X 좌표.
     * @param myVmY 현재 자판기의 Y 좌표.
     * @param totalOtherVmCount 현재 시스템의 다른 자판기 총 수 (응답 카운트 및 타임아웃 로직에 사용).
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
        int myVmY,
        int totalOtherVmCount
    );

    /**
     * @brief 자판기 시스템의 메인 실행 루프를 시작합니다.
     * 초기화 후, 현재 상태에 따라 적절한 작업을 수행하고 사용자 입력을 처리합니다.
     * 네트워크 이벤트는 별도의 io_context 스레드에서 처리됩니다.
     */
    void run();

private:
    // --- 의존성 주입된 객체들 (참조로 관리) ---
    presentation::UserInterface& userInterface_;
    service::InventoryService& inventoryService_;
    service::OrderService& orderService_;
    service::PrepaymentService& prepaymentService_;
    service::MessageService& messageService_;
    service::DistanceService& distanceService_;
    service::ErrorService& errorService_;
    boost::asio::io_context& ioContext_; ///< 네트워크 I/O 및 타이머를 위한 Asio io_context.

    // --- 현재 자판기 고유 정보 ---
    std::string myVendingMachineId_; ///< 본 자판기의 ID (예: "T1")
    int myVendingMachineX_;          ///< 본 자판기의 X 좌표 (0-99)
    int myVendingMachineY_;          ///< 본 자판기의 Y 좌표 (0-99)
    const int total_other_vms_;      ///< 나를 제외한 다른 자판기의 총 수 (응답 카운트용)

    // --- 현재 거래의 동적 상태 정보 (뮤텍스 mtx_로 보호) ---
    ControllerState currentState_;                          ///< 컨트롤러의 현재 상태
    std::optional<domain::Order> currentActiveOrder_;       ///< 현재 처리 중인 주문 정보
    bool isCurrentOrderPrepayment_ = false;                 ///< 현재 주문이 선결제인지 여부
    std::optional<domain::Drink> pendingDrinkSelection_;    ///< 사용자가 선택한 음료 정보 (주문 확정 전)
    std::vector<service::OtherVendingMachineInfo> availableOtherVmsForDrink_; ///< 다른 자판기 재고 조회 결과
    std::optional<domain::VendingMachine> selectedTargetVmForPrepayment_; ///< 선결제 대상 자판기 정보
    std::optional<service::ErrorInfo> last_error_info_; ///< 콜백/타이머에서 발생한 오류를 메인 스레드로 전달하기 위함

    // --- 동기화 객체 ---
    std::mutex mtx_; ///< 공유 데이터(위의 상태 및 데이터 변수들) 접근을 위한 뮤텍스
    std::condition_variable cv_; ///< 특정 조건(네트워크 응답 수신, 타임아웃) 대기를 위한 조건 변수
    bool cv_data_ready_ = false; ///< 조건 변수 알림을 위한 플래그 (응답/타임아웃 발생 시 true)

    // --- 타임아웃 처리용 Asio 타이머 ---
    boost::asio::steady_timer response_timer_; ///< 네트워크 응답 대기용 타이머
    std::chrono::seconds current_timeout_duration_; ///< 현재 타이머의 대기 시간 (초 단위)


    // --- 내부 상태 처리 및 유틸리티 메소드 ---
    void processCurrentState();
    void changeState(ControllerState newState); // 상태 변경 시 뮤텍스 사용
    void resetCurrentTransactionState();      // 거래 관련 상태 초기화 (뮤텍스 사용)
    domain::Drink getDrinkDetails(const std::string& drinkCode); // 음료 정보 조회

    // --- 초기화 ---
    void initializeSystemAndRegisterMessageHandlers();

    // --- 각 유스케이스 단계별 상태 처리 메소드 (private) ---
    void state_displayingMainMenu();            // UC1
    void state_awaitingDrinkSelection();        // UC2, UC3
    void state_awaitingPaymentConfirmation();   // UC4, UC11
    void state_processingPayment();             // UC4, UC5, UC6
    void state_dispensingDrink();               // UC7, UC14
    void state_broadcastingStockRequest();      // UC8
    // AWAITING_STOCK_RESPONSES는 processCurrentState에서 cv_.wait_for로 처리
    void state_displayingOtherVmOptions();      // UC10, UC11
    void state_issuingAuthCodeAndRequestingReservation(); // UC16
    // ISSUING_AUTH_CODE_AND_REQUESTING_RESERVATION 응답 대기도 processCurrentState에서 cv_.wait_for로 처리
    void state_displayingAuthCodeInfo();        // UC12
    void state_awaitingAuthCodeInputPrompt();   // UC13, UC14
    void state_transactionCompletedReturnToMenu();
    void state_handlingError(const service::ErrorInfo& errorInfo);

    // --- Asio 타이머 관련 헬퍼 ---
    void startResponseTimer(std::chrono::seconds duration, ControllerState stateToWatchOnTimeout);
    void handleTimeout(const boost::system::error_code& ec, ControllerState expectedStateDuringTimeout);

    // --- MessageService로부터 호출될 콜백 핸들러들 (io_context 스레드에서 실행) ---
    void onReqStockReceived(const network::Message& msg);   // UC17
    void onRespStockReceived(const network::Message& msg);  // UC9
    void onReqPrepayReceived(const network::Message& msg);  // UC15
    void onRespPrepayReceived(const network::Message& msg); // UC16
};

} // namespace service
