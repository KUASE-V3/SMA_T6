#include "service/ErrorService.hpp"
#include <string>

namespace service {

// ErrorType에 따라 기본 사용자 친화적 메시지를 반환합니다.
std::string ErrorService::getDefaultUserMessageForError(ErrorType type, const std::string& context) {
    std::string message;
    switch (type) {
        // --- 사용자 입력 오류 ---
        case ErrorType::INVALID_MENU_CHOICE:
            message = "잘못된 메뉴를 선택하셨습니다. 다시 입력해주세요.";
            break;
        case ErrorType::INVALID_DRINK_CODE_FORMAT: // 사용자가 음료 코드 직접 입력 시
            message = "음료 코드 형식이 올바르지 않습니다. 확인 후 다시 입력해주세요.";
            break;
        case ErrorType::AUTH_CODE_INVALID_FORMAT:
            // UC13 Exceptional E1: "형식이 맞지 않는 코드가 입력되면 시스템은 에러메시지를 호출해 보여준다." [cite: 28]
            message = "인증 코드 형식이 올바르지 않습니다 (5자리 영숫자). 다시 입력해주세요.";
            break;
        case ErrorType::USER_RESPONSE_TIMEOUT:
             // UC6 Exceptional E1: "30초 이상 사용자 응답이 오지 않으면 시스템은 에러메시지를 호출해 보여준다." (음료 선택 단계) [cite: 6]
             // UC11 Exceptional: "사용자가 10초 이내에 선결제 여부를 응답하지 않으면 시스템은 에러메시지를 호출해 보여준다." [cite: 24]
            message = "응답 시간이 초과되었습니다. (" + context + ")"; // context에 초 정보 등 포함 가능
            break;

        // --- 네트워크 통신 오류 ---
        case ErrorType::NETWORK_COMMUNICATION_ERROR:
            // UC8 Exceptional E1: "통신 네트워크에 문제가 발생하여 재고 요청이 실패하는 경우, 에러메시지를 호출해 보여준다" [cite: 18]
            message = "다른 자판기와의 통신 중 오류가 발생했습니다: " + context;
            break;
        case ErrorType::INVALID_MESSAGE_FORMAT:
            // UC17 Exceptional E1: "메시지를 읽을 수 없으면 시스템은 에러를 호출하고 무응답한다." [cite: 36]
            // UC9 Exceptional E1: "응답 메시지가 손상되었거나 유효하지 않은 경우, 해당 응답은 무시된다." [cite: 19]
            message = "다른 자판기로부터 잘못된 형식의 메시지를 수신했습니다: " + context;
            break;
        case ErrorType::RESPONSE_TIMEOUT_FROM_OTHER_VM:
            // UC9 Exceptional E2: "30초 이내에 아무런 응답 메시지가 오지 않는 경우 시스템은 에러메시지를 호출해 보여준다." [cite: 19]
            // UC4 Exceptional E1: "30초 이상 카드사로부터 응답이 오지 않으면 시스템은 에러메시지를 호출해 보여준다." (카드 결제) [cite: 10]
            // -> 카드 결제 타임아웃은 PAYMENT_PROCESSING_FAILED로 통합
            message = "다른 자판기(" + context + ")로부터 응답을 받지 못했습니다.";
            break;
        case ErrorType::STOCK_RESERVATION_FAILED_AT_OTHER_VM: // 선결제 시 재고 확보 요청 실패 응답
             // UC16 Exceptional E1: "응답 msg_content.availability == F 시 시스템은 에러메시지를 호출해 보여준다." [cite: 34]
            message = "다른 자판기(" + context + ")에서 음료 재고 확보에 실패했습니다.";
            break;


        // --- 카드 결제 오류 --- (단순 실패)
        case ErrorType::PAYMENT_PROCESSING_FAILED:
            message = "카드 결제에 실패했습니다. 잠시 후 다시 시도해주세요.";
            break;

        // --- 기타/내부 시스템 오류 (메인 메뉴로 돌아감) ---
        case ErrorType::DRINK_NOT_FOUND: // 사용자가 선택한 음료가 목록에 없는 경우 (로컬 자판기 확인 시)
            // UC3 Exceptional E1: "재고 정보를 불러오지 못했거나 데이터가 손상된 경우, 시스템은 에러메시지를 호출해 보여준다." [cite: 8]
            // 이 경우는 더 구체적인 'REPOSITORY_ACCESS_ERROR' 등으로 처리될 수 있으나,
            // 사용자가 존재하지 않는 음료 코드를 입력한 경우로 본다면 'DRINK_NOT_FOUND'가 적절.
            // 여기서는 사용자가 유효한 목록에서 선택한다고 가정하고, 이 오류는 내부적 문제로 간주.
            message = "선택하신 음료 정보를 처리하는 중 문제가 발생했습니다."; // 또는 "선택하신 음료를 찾을 수 없습니다."
            break;
        case ErrorType::DRINK_OUT_OF_STOCK:
            // UC3: "재고가 없거나 이거나 판매대상이 아닌 경우, 시스템은 BroadcastMessage 준비 후 인근 자판기에서의 탐색정보로 전환한다(UC8)" [cite: 8]
            // 이 경우는 오류라기보다 정상 흐름의 분기. 만약 다른 자판기 조회도 실패하면 그때 오류 처리.
            // 여기서는 다른 자판기 조회 옵션이 없는 단순 재고 부족 시 오류로 간주.
            message = "선택하신 음료(" + context + ")의 재고가 없습니다.";
            break;
        case ErrorType::AUTH_CODE_NOT_FOUND:
             // UC14 Exceptional E1: "시스템에 저장되 있지 않은 인증코드이면, 시스템은 에러메시지를 호출해 보여준다." [cite: 30]
            message = "입력하신 인증 코드를 찾을 수 없습니다.";
            break;
        case ErrorType::AUTH_CODE_ALREADY_USED:
             // UC14 Exceptional E1: "이미 사용된 인증코드이거나..." [cite: 30]
            message = "이미 사용된 인증 코드입니다.";
            break;
        case ErrorType::UNEXPECTED_SYSTEM_ERROR: // 그 외 모든 예측 못한 내부 오류
            message = "시스템 처리 중 예상치 못한 오류가 발생했습니다: " + context;
            break;
        case ErrorType::REPOSITORY_ACCESS_ERROR:
             // UC3 Exceptional E1: "재고 정보를 불러오지 못했거나 데이터가 손상된 경우, 시스템은 에러메시지를 호출해 보여준다." [cite: 8]
            message = "데이터 처리 중 오류가 발생했습니다: " + context;
            break;


        default: // ErrorType에 정의되지 않은 다른 오류들 (혹시 있다면)
            message = "알 수 없는 오류가 발생했습니다.";
            if (!context.empty()) {
                message += " (" + context + ")";
            }
            break;
    }
    return message;
}

// ErrorType에 따라 ErrorResolutionLevel을 반환합니다.
ErrorResolutionLevel ErrorService::getDefaultResolutionLevelForError(ErrorType type) {
    switch (type) {
        // === 사용자 입력 재시도 ===
        case ErrorType::INVALID_MENU_CHOICE:
        case ErrorType::INVALID_DRINK_CODE_FORMAT:
        case ErrorType::AUTH_CODE_INVALID_FORMAT:
            return ErrorResolutionLevel::RETRY_INPUT;

        // === 그 외 모든 경우는 메인 메뉴로 돌아가기 ===
        case ErrorType::USER_RESPONSE_TIMEOUT:
        case ErrorType::NETWORK_COMMUNICATION_ERROR:
        case ErrorType::INVALID_MESSAGE_FORMAT:
        case ErrorType::RESPONSE_TIMEOUT_FROM_OTHER_VM:
        case ErrorType::STOCK_RESERVATION_FAILED_AT_OTHER_VM:
        case ErrorType::PAYMENT_PROCESSING_FAILED:
        case ErrorType::DRINK_NOT_FOUND:
        case ErrorType::DRINK_OUT_OF_STOCK:
        case ErrorType::AUTH_CODE_NOT_FOUND:
        case ErrorType::AUTH_CODE_ALREADY_USED:
        case ErrorType::UNEXPECTED_SYSTEM_ERROR:
        case ErrorType::REPOSITORY_ACCESS_ERROR:
        // ErrorType에 정의되지 않은 다른 오류들도 일단 메인 메뉴로
        default:
            return ErrorResolutionLevel::RETURN_TO_MAIN_MENU;
    }
}

ErrorInfo ErrorService::processOccurredError(ErrorType occurredErrorType, const std::string& additionalContext) {
    std::string userMsg = getDefaultUserMessageForError(occurredErrorType, additionalContext);
    ErrorResolutionLevel resolution = getDefaultResolutionLevelForError(occurredErrorType);

    return ErrorInfo(occurredErrorType, userMsg, resolution);
}

} // namespace service