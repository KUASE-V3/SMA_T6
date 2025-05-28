#include "service/PrepaymentService.hpp"
#include "persistence/prepayCodeRepository.h"
#include "persistence/OrderRepository.hpp"   
#include "domain/prepaymentCode.h"
#include "domain/order.h"
#include "service/ErrorService.hpp"

#include <random>    
#include <string>
#include <algorithm> 
#include <memory>    // std::make_shared
#include <stdexcept> // Repository에서 발생 가능한 예외 처리용

namespace service {

/**
 * @brief PrepaymentService 생성자.
 * 의존성 주입을 통해 필요한 리포지토리 및 서비스 객체를 초기화합니다.
 */
PrepaymentService::PrepaymentService(
    persistence::PrepayCodeRepository& prepayCodeRepo,
    persistence::OrderRepository& orderRepo,
    service::ErrorService& errorService
) : prepayCodeRepository_(prepayCodeRepo),
    orderRepository_(orderRepo),
    errorService_(errorService) {}

/**
 * @brief 랜덤한 5자리 영숫자 인증 코드 문자열을 생성하여 반환합니다. (UC12)
 * (PFR R4.2: 사용자가 선결제를 요청할 때마다 시스템은 새로운 인증코드를 생성해야 한다.)
 * @return 생성된 5자리 인증 코드 문자열.
 */
std::string PrepaymentService::generateAuthCodeString() {
    // UC12 Typical Course 1: AuthCode 5자리로 랜덤하게 생성한다.
    return generateRandomAlphanumericString(5); // PFR: 5자리 영숫자
}

/**
 * @brief 생성된 인증 코드와 연결될 주문 정보를 받아 PrePaymentCode 객체를 생성하고 저장합니다. (UC12)
 * @param authCode `generateAuthCodeString()`으로 생성된 인증 코드.
 * @param order 이 선결제와 연결될 주문 객체의 `std::shared_ptr`.
 */
void PrepaymentService::registerPrepayment(const std::string& authCode, std::shared_ptr<domain::Order> order) {
    // 이 함수는 OrderService::processOrderApproval에서 선결제 시 호출됨.
    // UC5 (결제 승인) -> UC12 (인증코드 발급) -> 이 함수 호출 -> UC16 (재고 확보 요청)
    if (!order) {
        errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "선결제 등록 시 주문 정보(order)가 null입니다.");
        return;
    }
    try {
        domain::PrePaymentCode newPrepaymentCode(authCode, domain::CodeStatus::ACTIVE, order);
        prepayCodeRepository_.save(newPrepaymentCode);
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "선결제 정보 저장 중 오류: " + std::string(e.what()));
    }
}

/**
 * @brief 입력된 인증 코드의 형식이 유효한지 (5자리 영숫자) 검사합니다. (UC13)
 * (UC13 Typical Course 3: 입력 문자열이 정규식 “[A-Za-z0-9]{5}”와 일치하는지 검사한다.)
 * @param authCode 검사할 인증 코드 문자열.
 * @return 형식이 유효하면 true, 아니면 false.
 */
bool PrepaymentService::isValidAuthCodeFormat(const std::string& authCode) const {
    if (authCode.length() != 5) { // PFR: 5자리
        return false;
    }
    // PFR: 알파벳 대/소문자와 숫자 포함
    return std::all_of(authCode.begin(), authCode.end(), ::isalnum);
}

/**
 * @brief 입력된 인증 코드가 저장되어 있고 사용 가능한지(ACTIVE 상태) 확인합니다. (UC14)
 * (UC14 Typical Course 1: 저장하고 있는 코드와 입력 코드가 일치하는지 확인한다.)
 * @param authCode 검증할 인증 코드 문자열.
 * @return 유효하고 사용 가능한 경우 해당 domain::PrePaymentCode 객체.
 * 그렇지 않거나 오류 발생 시 기본 생성된(코드가 비어있는) PrePaymentCode 객체 반환.
 */
domain::PrePaymentCode PrepaymentService::getPrepaymentDetailsIfActive(const std::string& authCode) {
    try {
        domain::PrePaymentCode prepayCode = prepayCodeRepository_.findByCode(authCode);

        if (prepayCode.getCode().empty()) { // findByCode가 못 찾으면 기본 객체 반환
            // UC14 E1: 시스템에 저장되어 있지 않은 인증코드
            errorService_.processOccurredError(ErrorType::AUTH_CODE_NOT_FOUND, "인증 코드(" + authCode + ")를 찾을 수 없습니다.");
            return domain::PrePaymentCode(); // 빈 객체 반환
        }

        if (!prepayCode.isUsable()) { // isUsable()은 status가 ACTIVE인지 확인
            // UC14 E1: 이미 사용된 인증코드
            errorService_.processOccurredError(ErrorType::AUTH_CODE_ALREADY_USED, "이미 사용된 인증 코드(" + authCode + ")입니다.");
            return domain::PrePaymentCode(); // 빈 객체 반환
        }
        return prepayCode; // 유효하고 사용 가능한 코드
    } catch (const std::exception& e) { // Repository 접근 등 예외
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "선결제 상세 정보 조회 중 시스템 오류: " + std::string(e.what()));
        return domain::PrePaymentCode();
    }
}

/**
 * @brief 인증 코드의 상태를 "USED"로 변경합니다. (UC14)
 * (UC14 Typical Course 2: 일치할 경우 AuthCode를 만료시킨다.)
 * @param authCode 사용 완료 처리할 인증 코드 문자열.
 */
void PrepaymentService::changeAuthCodeStatusToUsed(const std::string& authCode) {
    try {
        bool success = prepayCodeRepository_.updateStatus(authCode, domain::CodeStatus::USED);
        if (!success) {
            // updateStatus가 false를 반환하는 경우는 코드가 없거나, 이미 USED 상태였거나 등.
            errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드(" + authCode + ") 상태를 USED로 변경 실패 (찾을 수 없거나 이미 처리됨)");
        }
    } catch (const std::exception& e) { // Repository 접근 등 예외
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "인증 코드 상태 변경 중 시스템 오류: " + std::string(e.what()));
    }
}

/**
 * @brief 다른 자판기로부터 수신한 선결제 요청 정보를 바탕으로,
 * PrePaymentCode 객체와 관련 Order 객체를 생성하고 저장합니다. (UC15)
 * @param certCode 수신한 인증 코드.
 * @param drinkCode 수신한 음료 코드.
 * @param vmidForOrder 이 선결제 건을 위해 생성될 Order에 기록될 자판기 ID (요청을 보낸 자판기의 ID).
 */
void PrepaymentService::recordIncomingPrepayment(const std::string& certCode, const std::string& drinkCode, const std::string& vmidForOrder) {
    // 이 함수는 UserProcessController::onReqPrepayReceived에서 호출됨.
    // 재고 차감은 InventoryService에서 이미 수행된 후 호출됨.
    try {
        // 1. 이 선결제 건에 대한 Order 객체 생성 (상태: APPROVED, 선결제 완료된 주문으로 간주)
        //    이 Order는 다른 자판기에서 발생한 주문에 대한 '우리 자판기에서의 기록'임.
        auto orderForThisPrepayment = std::make_shared<domain::Order>(
            vmidForOrder,       // 주문 발생 자판기 ID (요청 보낸 자판기)
            drinkCode,          // 주문된 음료 코드
            1,                  // 수량은 1로 고정 (PFR 메시지 포맷의 item_num은 이 함수 호출 전에 UserProcessController에서 처리)
            certCode,           // 수신된 인증 코드
            "APPROVED"          
        );
        orderRepository_.save(*orderForThisPrepayment); // 우리 자판기의 OrderRepository에 저장

        // 2. PrePaymentCode 객체 생성 및 저장 (상태: ACTIVE)
        domain::PrePaymentCode newPrepaymentCode(certCode, domain::CodeStatus::ACTIVE, orderForThisPrepayment);
        prepayCodeRepository_.save(newPrepaymentCode);

    } catch (const std::exception& e) { // Repository 접근 등 예외
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "수신된 선결제 정보 등록 중 시스템 오류: " + std::string(e.what()));
    }
}

/**
 * @brief 지정된 길이의 랜덤 영숫자 문자열을 생성하는 내부 헬퍼 함수입니다.
 * @param length 생성할 문자열의 길이.
 * @return 생성된 랜덤 영숫자 문자열.
 */
std::string PrepaymentService::generateRandomAlphanumericString(size_t length) const {
    static const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 2); 
    
    std::random_device rd;
    std::mt19937 generator(rd()); 
    std::uniform_int_distribution<size_t> distribution(0, max_index);

    std::string random_string;
    random_string.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        random_string += charset[distribution(generator)];
    }
    return random_string;
}

} // namespace service
