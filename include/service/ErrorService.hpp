// #pragma once

// #include <string>
// #include <functional> // std::function 사용 가능성 (컨트롤러 직접 호출 시)

// // Forward declaration (UserProcessController와의 상호작용을 위해)
// namespace service { class UserProcessController; } // UserProcessController도 service 네임스페이스 가정

// namespace service {

// // 발생 가능한 구체적인 오류 유형들
// // (예시이며, 실제 서비스 구현 시 더 구체화/추가/수정 필요)
// enum class ErrorType {
//     // Inventory Errors
//     INSUFFICIENT_STOCK,
//     DRINK_NOT_FOUND_IN_INVENTORY,

//     // Repository/Data Errors
//     ITEM_NOT_FOUND_IN_DB, // 예: Order, PrePaymentCode 등 조회 실패
//     DATA_ACCESS_FAILURE,  // 예: 파일/DB 읽기/쓰기 실패
//     DATA_CORRUPTION,

//     // Payment Errors
//     PAYMENT_PROCESSING_FAILED,
//     PAYMENT_TIMEOUT,
//     INVALID_PAYMENT_RESPONSE,

//     // Prepayment/AuthCode Errors
//     AUTH_CODE_INVALID_FORMAT,
//     AUTH_CODE_NOT_FOUND,
//     AUTH_CODE_ALREADY_USED,
//     STOCK_RESERVATION_FAILED_AT_OTHER_VM, // 선결제 시 상대방 VM 재고 확보 실패

//     // Network Errors
//     NETWORK_COMMUNICATION_ERROR,
//     MESSAGE_SEND_FAILED,
//     MESSAGE_RECEIVE_FAILED,
//     INVALID_MESSAGE_FORMAT, // 수신 메시지 파싱 실패
//     RESPONSE_TIMEOUT,

//     // User Input/Interaction Errors
//     USER_RESPONSE_TIMEOUT, // 사용자 응답 시간 초과
//     INVALID_USER_INPUT,    // 사용자의 잘못된 입력 (예: 메뉴 선택 범위 초과)

//     // General/System Errors
//     INITIALIZATION_FAILED,
//     UNKNOWN_SYSTEM_ERROR
// };

// // 오류 처리 후 UserProcessController가 취해야 할 행동 수준
// // 이 수준에 따라 UserProcessController는 사용자에게 메시지를 보여주고,
// // 이전 단계로 돌아가거나, 메인 메뉴로 돌아가거나, 시스템을 초기화하는 등의 작업을 수행.
// enum class ErrorResolutionLevel {
//     RETRY_CURRENT_STEP,       // 현재 단계 재시도 (가능한 경우)
//     RETURN_TO_PREVIOUS_STEP,  // 이전 단계로 돌아가기
//     RETURN_TO_MAIN_MENU,      // 메인 메뉴(음료 목록)로 돌아가기
//     NOTIFY_AND_CONTINUE,      // 사용자에게 알리고 계속 진행 (경미한 오류)
//     SYSTEM_RESET_OR_HALT      // 심각한 오류로 시스템 초기화 또는 중단 필요
// };

// // ErrorService에 전달될 오류 정보
// struct ErrorInfo {
//     ErrorType type;                     // 발생한 구체적인 오류 유형
//     std::string userFriendlyMessage;    // 사용자에게 보여줄 수 있는 (기본) 메시지
//     ErrorResolutionLevel resolutionLevel; // 이 오류에 대해 권장되는 해결 수준

//     ErrorInfo(ErrorType t, std::string msg, ErrorResolutionLevel level)
//         : type(t), userFriendlyMessage(std::move(msg)), resolutionLevel(level) {}
// };

// class ErrorService {
// public:
//     // UserProcessController가 ErrorService로부터 ErrorInfo를 받아 스스로 결정
//     ErrorService() = default;

//     // 서비스나 다른 컴포넌트에서 오류 발생 시 이 함수를 호출.
//     // 이 함수는 발생한 오류 타입에 따라 적절한 ErrorInfo 객체를 생성하여 반환.
//     // UserProcessController는 이 ErrorInfo를 받아 후속 조치를 결정.
//     ErrorInfo processOccurredError(ErrorType 발생한_오류_타입, const std::string& 추가_메시지_또는_컨텍스트 = "");

//     // (선택적) 자주 사용되는 오류에 대한 헬퍼 함수들
//     // ErrorInfo reportInsufficientStock(const std::string& drinkName, int requested, int available);
//     // ErrorInfo reportItemNotFound(const std::string& itemName);

// private:
//     // 내부적으로 ErrorType에 따라 적절한 userFriendlyMessage와 ErrorResolutionLevel을 매핑하거나 결정하는 로직
//     std::string getDefaultUserMessageForError(ErrorType type);
//     ErrorResolutionLevel getDefaultResolutionLevelForError(ErrorType type);
// };

// } // namespace service

