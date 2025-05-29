#pragma once

#include <string>

namespace service {

/**
 * @brief 시스템에서 발생할 수 있는 구체적인 오류 유형들을 정의합니다.
 * 각 ErrorType은 관련된 유스케이스(UC)의 예외 상황(E)과 매핑될 수 있습니다.
 */
enum class ErrorType {
    // === 사용자 입력/상호작용 오류 ===
    INVALID_MENU_CHOICE,          // 잘못된 메뉴 선택 (예: UC1 E1 - 메뉴 형식 오류 시)
    INVALID_DRINK_CODE_FORMAT,    // 음료 코드 형식 오류 (예: UC2에서 사용자가 코드를 직접 입력 시)
    AUTH_CODE_INVALID_FORMAT,     // 인증 코드 형식 오류 (UC13 E1)
    USER_RESPONSE_TIMEOUT,        // 사용자 응답 시간 초과 (UC2 E1, UC4 E1, UC11 E1)

    // === 네트워크/메시지 오류 ===
    NETWORK_COMMUNICATION_ERROR,  // 일반 통신 오류 (UC8 E1)
    MESSAGE_SEND_FAILED,          // 메시지 전송 실패
    MESSAGE_RECEIVE_FAILED,       // 메시지 수신 실패
    INVALID_MESSAGE_FORMAT,       // 수신 메시지 파싱/형식 오류 (UC9 E1, UC17 E1)
    RESPONSE_TIMEOUT_FROM_OTHER_VM, // 다른 자판기 응답 시간 초과 (UC9 E2)
    STOCK_RESERVATION_FAILED_AT_OTHER_VM, // 다른 자판기 선결제 재고 확보 실패 응답 (UC16 E1)

    // === 카드 결제 오류 ===
    PAYMENT_PROCESSING_FAILED,    // 카드 결제 실패 (UC4 E2, 카드사 DECLINED 포함)
    PAYMENT_TIMEOUT,              // 카드사 응답 시간 초과 (UC4 E1)
    PAYMENT_SYSTEM_UNAVAILABLE,   // 외부 결제 시스템 연동 실패

    // === 시스템/데이터/내부 로직 오류 ===
    DRINK_NOT_FOUND,              // 요청한 음료 정보를 찾을 수 없음 (UC1 E1, UC3)
    DRINK_OUT_OF_STOCK,           // 현재 자판기 재고 없음 (UC3)
    INSUFFICIENT_STOCK_FOR_DECREASE, // 재고 차감 시도 시 실제 재고 부족
    AUTH_CODE_NOT_FOUND,          // 존재하지 않는 인증 코드 (UC14 E1)
    AUTH_CODE_ALREADY_USED,       // 이미 사용된 인증 코드 (UC14 E1)
    AUTH_CODE_GENERATION_FAILED,  // 인증 코드 생성 실패 (UC12 관련)
    REPOSITORY_ACCESS_ERROR,      // 데이터 저장소 접근 오류 (UC1 E1, UC3 E1)
    UNEXPECTED_SYSTEM_ERROR,      // 그 외 모든 예측 못한 내부 시스템 오류
    INITIALIZATION_FAILED         // 시스템 시작 시 초기화 실패
};

/**
 * @brief 오류 처리 후 UserProcessController가 취해야 할 행동 수준을 정의합니다.
 */
enum class ErrorResolutionLevel {
    RETRY_INPUT,              // 사용자에게 현재 입력 단계 재시도 요청
    RETURN_TO_MAIN_MENU,      // 메인 메뉴(음료 목록 표시)로 돌아가기
    SYSTEM_FATAL_ERROR        // 복구 불가능한 심각한 오류
};

/**
 * @brief ErrorService가 UserProcessController에게 전달할 오류 정보 및 처리 지침.
 */
struct ErrorInfo {
    ErrorType type;                     // 발생한 오류의 구체적인 타입
    std::string userFriendlyMessage;    // 사용자에게 보여줄 메시지
    ErrorResolutionLevel resolutionLevel; // 이 오류에 대해 권장되는 해결 수준

    /**
     * @brief ErrorInfo 객체를 생성합니다.
     * @param t 발생한 오류 타입.
     * @param msg 사용자에게 보여줄 메시지.
     * @param level 권장 해결 수준.
     */
    ErrorInfo(ErrorType t, std::string msg, ErrorResolutionLevel level)
        : type(t), userFriendlyMessage(std::move(msg)), resolutionLevel(level) {}
};

/**
 * @brief 시스템 전반의 오류를 중앙에서 처리하고,
 * UserProcessController에게 적절한 사용자 메시지와 후속 조치 수준을 제공하는 서비스.
 */
class ErrorService {
public:
    ErrorService() = default;

    /**
     * @brief 발생한 오류 타입과 추가적인 문맥 정보를 바탕으로 ErrorInfo 객체를 생성하여 반환합니다.
     * @param occurredErrorType 발생한 구체적인 오류의 타입.
     * @param additionalContext (선택적) 오류 메시지에 포함될 추가적인 문맥 정보 (예: 음료 이름).
     * @return 생성된 ErrorInfo 객체 (오류 타입, 사용자 메시지, 해결 수준 포함).
     */
    ErrorInfo processOccurredError(ErrorType occurredErrorType, const std::string& additionalContext = "");

private:
    std::string getDefaultUserMessageForError(ErrorType type, const std::string& context);
    ErrorResolutionLevel getDefaultResolutionLevelForError(ErrorType type);
};

} // namespace service
