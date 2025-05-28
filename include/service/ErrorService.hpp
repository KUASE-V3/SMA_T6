#pragma once

#include <string>
#include <map> // ErrorInfo의 contextDetails 용도 (선택적) - 현재 사용 안 함

namespace service {

// 발생 가능한 구체적인 오류 유형들 (지속적으로 업데이트)
enum class ErrorType {
    // === 사용자 입력/상호작용 오류 ===
    INVALID_MENU_CHOICE,          // 잘못된 메뉴 선택
    INVALID_DRINK_CODE_FORMAT,    // 음료 코드 형식 오류 (사용자가 직접 입력하는 경우)
    AUTH_CODE_INVALID_FORMAT,     // 인증 코드 형식 오류 [cite: 28]
    USER_RESPONSE_TIMEOUT,        // 사용자 응답 시간 초과 [cite: 6, 24]

    // === 네트워크/메시지 오류 ===
    NETWORK_COMMUNICATION_ERROR,  // 일반 통신 오류 [cite: 18]
    // MESSAGE_SEND_FAILED,       // (NETWORK_COMMUNICATION_ERROR로 통합 가능)
    // MESSAGE_RECEIVE_FAILED,    // (NETWORK_COMMUNICATION_ERROR로 통합 가능)
    INVALID_MESSAGE_FORMAT,       // 수신 메시지 파싱/형식 오류 [cite: 19, 36]
    RESPONSE_TIMEOUT_FROM_OTHER_VM, // 다른 자판기 응답 시간 초과 [cite: 19]
    STOCK_RESERVATION_FAILED_AT_OTHER_VM, // 다른 자판기 선결제 재고 확보 실패 응답 [cite: 34]

    // === 카드 결제 오류 ===
    PAYMENT_PROCESSING_FAILED,    // 카드 결제 실패 (외부 시스템(시뮬레이션) 응답이 '실패'인 경우) [cite: 10]

    // === 기타 시스템/내부 오류 (주로 메인 메뉴로 복귀) ===
    DRINK_NOT_FOUND,              // 선택한 음료가 목록에 없음 (내부 조회 실패)
    DRINK_OUT_OF_STOCK,           // 현재 자판기 재고 없음 (다른 자판기 조회 전 또는 최종 실패 시)
    AUTH_CODE_NOT_FOUND,          // 존재하지 않는 인증 코드 [cite: 30]
    AUTH_CODE_ALREADY_USED,       // 이미 사용된 인증 코드 [cite: 30]
    // AUTH_CODE_GENERATION_FAILED, // (UNEXPECTED_SYSTEM_ERROR로 통합 가능)
    REPOSITORY_ACCESS_ERROR,      // 데이터 저장소 접근 중 오류 [cite: 8]
    UNEXPECTED_SYSTEM_ERROR       // 예상치 못한 내부 시스템 오류
    // INITIALIZATION_FAILED       // (UNEXPECTED_SYSTEM_ERROR로 통합 가능 또는 별도 처리)
};

// 오류 처리 후 UserProcessController가 취해야 할 행동 수준
enum class ErrorResolutionLevel {
    RETRY_INPUT,              // 사용자에게 입력 재시도 요청 (현재 상태 유지)
    RETURN_TO_MAIN_MENU,      // 메인 메뉴(음료 목록 표시)로 돌아가기
    // NOTIFY_AND_STAY,       // (현재 요구사항에서는 사용 안 함) 사용자에게 알리고 현재 상태 유지
    // SYSTEM_FATAL_ERROR     // (현재 요구사항에서는 사용 안 함) 복구 불가능, 시스템 중단
};

// ErrorService에 전달될 오류 정보 및 처리 결과
struct ErrorInfo {
    ErrorType type;
    std::string userFriendlyMessage; // 사용자에게 보여줄 메시지
    ErrorResolutionLevel resolutionLevel;
    // std::map<std::string, std::string> details; // (선택적) 오류 상세 정보 - 현재 사용 안 함

    ErrorInfo(ErrorType t, std::string msg, ErrorResolutionLevel level)
        : type(t), userFriendlyMessage(std::move(msg)), resolutionLevel(level) {}
};

class ErrorService {
public:
    ErrorService() = default;

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