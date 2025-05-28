#pragma once

#include <string>
#include <map> // ErrorInfo의 contextDetails 용도 (선택적)
// #include <chrono> // timestamp는 현재 제외

namespace service {

// 발생 가능한 구체적인 오류 유형들 (지속적으로 업데이트)
enum class ErrorType {
    // User Input/Interaction Errors
    INVALID_MENU_CHOICE,
    INVALID_DRINK_CODE_FORMAT, // 형식 오류 (예: 숫자가 아닌 값)
    USER_RESPONSE_TIMEOUT,

    // Inventory/Stock Errors
    DRINK_NOT_FOUND,        // 입력한 음료 코드가 전체 20종에 없음
    DRINK_OUT_OF_STOCK,     // 현재 자판기 재고 없음
    INSUFFICIENT_STOCK_FOR_DECREASE, // 재고 차감 시도 시 실제 재고 부족 (내부 오류 가능성)

    // Payment Errors
    PAYMENT_PROCESSING_FAILED,
    PAYMENT_TIMEOUT,
    PAYMENT_SYSTEM_UNAVAILABLE, // 외부 결제 시스템 연동 실패

    // Prepayment/AuthCode Errors
    AUTH_CODE_INVALID_FORMAT,   // 입력된 인증 코드 형식 오류
    AUTH_CODE_NOT_FOUND,        // 존재하지 않는 인증 코드
    AUTH_CODE_ALREADY_USED,     // 이미 사용된 인증 코드
    AUTH_CODE_GENERATION_FAILED,// 인증 코드 생성 실패 (내부 오류)
    STOCK_RESERVATION_FAILED_AT_OTHER_VM, // 다른 자판기 선결제 재고 확보 실패 응답

    // Network/Message Errors
    NETWORK_COMMUNICATION_ERROR,// 일반 통신 오류
    MESSAGE_SEND_FAILED,
    MESSAGE_RECEIVE_FAILED,
    INVALID_MESSAGE_FORMAT,     // 수신 메시지 파싱/형식 오류
    RESPONSE_TIMEOUT_FROM_OTHER_VM, // 다른 자판기 응답 시간 초과

    // System/Internal Errors
    REPOSITORY_ACCESS_ERROR,    // 레포지토리 접근 중 오류 (파일 I/O 등)
    UNEXPECTED_SYSTEM_ERROR,    // 예상치 못한 내부 시스템 오류
    INITIALIZATION_FAILED       // 시스템 초기화 실패
};

// 오류 처리 후 UserProcessController가 취해야 할 행동 수준
enum class ErrorResolutionLevel {
    RETRY_INPUT,              // 사용자에게 입력 재시도 요청 (현재 상태 유지)
    RETURN_TO_PREVIOUS_STEP,  // 이전 논리적 단계로 돌아가기
    RETURN_TO_MAIN_MENU,      // 메인 메뉴(음료 목록 표시)로 돌아가기
    NOTIFY_AND_STAY,          // 사용자에게 알리고 현재 상태 유지 (예: 단순 정보 메시지)
    SYSTEM_FATAL_ERROR        // 복구 불가능, 시스템 중단 또는 안전 모드 진입
};

// ErrorService에 전달될 오류 정보 및 처리 결과
struct ErrorInfo {
    ErrorType type;
    std::string userFriendlyMessage; // 사용자에게 보여줄 메시지
    ErrorResolutionLevel resolutionLevel;
    // std::map<std::string, std::string> details; // (선택적) 오류 상세 정보

    ErrorInfo(ErrorType t, std::string msg, ErrorResolutionLevel level)
        : type(t), userFriendlyMessage(std::move(msg)), resolutionLevel(level) {}
};

class ErrorService {
public:
    ErrorService() = default;
    // ~ErrorService() = default; // 상속 고려 안 함

    /**
     * @brief 발생한 오류 타입과 추가 정보를 바탕으로 ErrorInfo 객체를 생성하여 반환합니다.
     * UserProcessController는 이 ErrorInfo를 받아 후속 조치를 결정합니다.
     * @param occurredErrorType 발생한 구체적인 오류의 타입.
     * @param additionalContext (선택적) 오류 메시지에 포함될 추가적인 문맥 정보 (예: 음료 이름).
     * @return 생성된 ErrorInfo 객체.
     */
    ErrorInfo processOccurredError(ErrorType occurredErrorType, const std::string& additionalContext = "");

private:
    // ErrorType에 따라 기본 userFriendlyMessage와 ErrorResolutionLevel을 결정하는 내부 로직
    std::string getDefaultUserMessageForError(ErrorType type, const std::string& context);
    ErrorResolutionLevel getDefaultResolutionLevelForError(ErrorType type);
};

} // namespace service