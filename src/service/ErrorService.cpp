#include "service/ErrorService.hpp"
#include <string>

namespace service {

/**
 * @brief ErrorType과 추가 컨텍스트를 바탕으로 사용자에게 보여줄 기본 오류 메시지를 생성합니다.
 * @param type 발생한 오류의 타입.
 * @param context 오류 메시지에 포함될 추가적인 문맥 정보.
 * @return 사용자 친화적인 오류 메시지 문자열.
 */
std::string ErrorService::getDefaultUserMessageForError(ErrorType type, const std::string& context) {
    std::string message;
    std::string ctx_msg = context.empty() ? "" : " (" + context + ")";

    switch (type) {
        // === 사용자 입력/상호작용 오류 ===
        case ErrorType::INVALID_MENU_CHOICE:          // UC1 등
            message = "잘못된 메뉴를 선택하셨습니다. 다시 입력해주세요.";
            break;
        case ErrorType::INVALID_DRINK_CODE_FORMAT:    // UC2 등
            message = "음료 코드 형식이 올바르지 않습니다. 확인 후 다시 입력해주세요.";
            break;
        case ErrorType::AUTH_CODE_INVALID_FORMAT:     // UC13 E1
            message = "인증 코드 형식이 올바르지 않습니다 (5자리 영숫자). 다시 입력해주세요.";
            break;
        case ErrorType::USER_RESPONSE_TIMEOUT:        // UC2 E1, UC4 E1, UC11 E1
            message = "응답 시간이 초과되었습니다." + ctx_msg;
            break;

        // === 네트워크/메시지 오류 ===
        case ErrorType::NETWORK_COMMUNICATION_ERROR:  // UC8 E1 등
            message = "다른 자판기와의 통신 중 오류가 발생했습니다." + ctx_msg;
            break;
        case ErrorType::MESSAGE_SEND_FAILED:
            message = "메시지 전송에 실패했습니다." + ctx_msg;
            break;
        case ErrorType::MESSAGE_RECEIVE_FAILED:
            message = "메시지 수신에 실패했습니다." + ctx_msg;
            break;
        case ErrorType::INVALID_MESSAGE_FORMAT:       // UC9 E1, UC17 E1
            message = "다른 자판기로부터 잘못된 형식의 메시지를 수신했습니다." + ctx_msg;
            break;
        case ErrorType::RESPONSE_TIMEOUT_FROM_OTHER_VM: // UC9 E2
            message = "다른 자판기" + ctx_msg + "로부터 응답을 받지 못했습니다.";
            break;
        case ErrorType::STOCK_RESERVATION_FAILED_AT_OTHER_VM: // UC16 E1
            message = "다른 자판기" + ctx_msg + "에서 음료 재고 확보에 실패했습니다.";
            break;

        // === 카드 결제 오류 ===
        case ErrorType::PAYMENT_PROCESSING_FAILED:    // UC4 E2 등
            message = "카드 결제에 실패했습니다. 다시 시도해주세요.";
            break;
        case ErrorType::PAYMENT_TIMEOUT:              // UC4 E1
            message = "카드사 응답 시간이 초과되었습니다.";
            break;
        case ErrorType::PAYMENT_SYSTEM_UNAVAILABLE:
            message = "결제 시스템을 현재 사용할 수 없습니다. 관리자에게 문의해주세요.";
            break;

        // === 시스템/데이터/내부 로직 오류 ===
        case ErrorType::DRINK_NOT_FOUND:              // UC1 E1 등
            message = "요청하신 음료 정보를 찾을 수 없습니다." + ctx_msg;
            break;
        case ErrorType::DRINK_OUT_OF_STOCK:           // UC3 등
            message = "선택하신 음료는 현재 재고가 없습니다." + ctx_msg;
            break;
        case ErrorType::INSUFFICIENT_STOCK_FOR_DECREASE:
            message = "재고를 차감하는 중 실제 수량이 부족합니다." + ctx_msg;
            break;
        case ErrorType::AUTH_CODE_NOT_FOUND:          // UC14 E1
            message = "입력하신 인증 코드를 찾을 수 없습니다." + ctx_msg;
            break;
        case ErrorType::AUTH_CODE_ALREADY_USED:       // UC14 E1
            message = "이미 사용된 인증 코드입니다." + ctx_msg;
            break;
        case ErrorType::AUTH_CODE_GENERATION_FAILED:  // UC12 관련
            message = "인증 코드 생성에 실패했습니다. 잠시 후 다시 시도해주세요.";
            break;
        case ErrorType::REPOSITORY_ACCESS_ERROR:      // UC1 E1, UC3 E1 등
            message = "데이터 처리 중 오류가 발생했습니다." + ctx_msg;
            break;
        case ErrorType::INITIALIZATION_FAILED:
            message = "시스템 초기화에 실패했습니다." + ctx_msg;
            break;
        case ErrorType::UNEXPECTED_SYSTEM_ERROR:
        default:
            message = "시스템 내부 오류가 발생했습니다. 잠시 후 다시 시도해주세요." + ctx_msg;
            break;
    }
    return message;
}

/**
 * @brief ErrorType에 따라 UserProcessController가 취해야 할 기본 해결 수준을 결정합니다.
 * @param type 발생한 오류의 타입.
 * @return 권장되는 ErrorResolutionLevel.
 */
ErrorResolutionLevel ErrorService::getDefaultResolutionLevelForError(ErrorType type) {
    switch (type) {
        case ErrorType::INVALID_MENU_CHOICE:
        case ErrorType::INVALID_DRINK_CODE_FORMAT:
        case ErrorType::AUTH_CODE_INVALID_FORMAT:
            return ErrorResolutionLevel::RETRY_INPUT; // 사용자 입력 재시도 유도

        // 대부분의 다른 오류는 메인 메뉴로 돌아가서 사용자가 다시 시작하도록 함
        case ErrorType::USER_RESPONSE_TIMEOUT:
        case ErrorType::NETWORK_COMMUNICATION_ERROR:
        case ErrorType::MESSAGE_SEND_FAILED:
        case ErrorType::MESSAGE_RECEIVE_FAILED:
        case ErrorType::INVALID_MESSAGE_FORMAT:
        case ErrorType::RESPONSE_TIMEOUT_FROM_OTHER_VM:
        case ErrorType::STOCK_RESERVATION_FAILED_AT_OTHER_VM:
        case ErrorType::PAYMENT_PROCESSING_FAILED:
        case ErrorType::PAYMENT_TIMEOUT:
        case ErrorType::DRINK_NOT_FOUND:
        case ErrorType::DRINK_OUT_OF_STOCK:
        case ErrorType::AUTH_CODE_NOT_FOUND:
        case ErrorType::AUTH_CODE_ALREADY_USED:
        case ErrorType::AUTH_CODE_GENERATION_FAILED:
        case ErrorType::REPOSITORY_ACCESS_ERROR:
        case ErrorType::INSUFFICIENT_STOCK_FOR_DECREASE:
        case ErrorType::UNEXPECTED_SYSTEM_ERROR:
            return ErrorResolutionLevel::RETURN_TO_MAIN_MENU;

        // 시스템 운영 불가 수준의 심각한 오류
        case ErrorType::PAYMENT_SYSTEM_UNAVAILABLE:
        case ErrorType::INITIALIZATION_FAILED:
            return ErrorResolutionLevel::SYSTEM_FATAL_ERROR;

        default:
            return ErrorResolutionLevel::RETURN_TO_MAIN_MENU; // 알 수 없는 오류는 안전하게 처리
    }
}

ErrorInfo ErrorService::processOccurredError(ErrorType occurredErrorType, const std::string& additionalContext) {
    std::string userMsg = getDefaultUserMessageForError(occurredErrorType, additionalContext);
    ErrorResolutionLevel resolution = getDefaultResolutionLevelForError(occurredErrorType);
    return ErrorInfo(occurredErrorType, userMsg, resolution);
}

} // namespace service
