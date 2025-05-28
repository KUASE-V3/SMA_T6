#include "service/PrepaymentService.hpp"
#include "persistence/prepayCodeRepository.h"
#include "persistence/OrderRepository.hpp"
#include "domain/prepaymentCode.h" // domain::PrePaymentCode 직접 사용
#include "domain/order.h"         // domain::Order 직접 사용
#include "service/ErrorService.hpp"

#include <random>   // std::random_device, std::mt19937, std::uniform_int_distribution
#include <string>
#include <algorithm> // std::all_of, std::isalnum
#include <memory>    // std::make_shared

namespace service {

PrepaymentService::PrepaymentService(
    persistence::PrepayCodeRepository& prepayCodeRepo,
    persistence::OrderRepository& orderRepo,
    service::ErrorService& errorService
) : prepayCodeRepository_(prepayCodeRepo),
    orderRepository_(orderRepo),
    errorService_(errorService) {}

/**
 * @brief 랜덤한 5자리 영숫자 인증 코드 문자열을 생성합니다.
 * @return 생성된 인증 코드 문자열.
 */
std::string PrepaymentService::generateAuthCodeString() {
    // 요구사항: 인증코드는 알파벳 대/소문자와 숫자를 포함하는 랜덤한 5자리의 문자열 [cite: 82]
    // 요구사항: 사용자가 선결제를 요청할 때마다 시스템은 새로운 인증코드를 생성해야 한다 [cite: 82]
    // UC12: "(S): AuthCode 5자리로 랜덤하게 생성한다." [cite: 25]
    return generateRandomAlphanumericString(5);
}

/**
 * @brief 생성된 인증 코드 문자열과 관련 Order를 받아 PrePaymentCode 객체를 생성하고 저장합니다.
 * @param authCode generateAuthCodeString()으로 생성된 인증 코드.
 * @param order 이 선결제와 연결될 주문 객체의 shared_ptr.
 */
void PrepaymentService::registerPrepayment(const std::string& authCode, std::shared_ptr<domain::Order> order) {
    // UC12 (인증코드 발급)과 UC16 (재고 확보 요청 전송) 이후 실제 코드 발급/저장 단계
    // UC5 (결제 승인 처리) 이후 선결제인 경우 -> UC12 (인증코드 발급) -> 이 메소드 호출
    // UC16 (재고 확보 요청 전송) 후 "resp_prepay응답을 수신하면 인증코드 발급 및 위치 안내 단계를 수행한다 (UC12)" [cite: 34]
    // 이때 생성된 authCode와 order를 가지고 PrePaymentCode를 만들어 저장합니다.

    if (!order) {
        errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "선결제 등록 시 주문 정보가 유효하지 않습니다.");
        // 여기서 예외를 던지거나, 호출 측에서 ErrorInfo를 확인하도록 할 수 있습니다.
        // 헤더 주석에는 VendingMachineException을 던진다고 되어 있으나, 여기서는 ErrorService를 사용합니다.
        return;
    }

    try {
        domain::PrePaymentCode newPrepaymentCode(authCode, domain::CodeStatus::ACTIVE, order);
        prepayCodeRepository_.save(newPrepaymentCode);
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "선결제 정보 저장 중 오류: " + std::string(e.what()));
        // throw; // 필요시 예외를 다시 던짐
    }
}

/**
 * @brief 입력된 인증 코드의 형식이 유효한지 (예: 5자리 영숫자) 검사합니다.
 * @param authCode 검사할 인증 코드 문자열.
 * @return 형식이 유효하면 true, 아니면 false.
 */
bool PrepaymentService::isValidAuthCodeFormat(const std::string& authCode) const {
    // UC13: "(S): 입력 문자열이 정규식 “[A-Za-z0-9]{5}""와 일치하는지 검사한다." [cite: 28]
    if (authCode.length() != 5) {
        return false;
    }
    return std::all_of(authCode.begin(), authCode.end(), ::isalnum);
}

/**
 * @brief 입력된 인증 코드가 저장되어 있고 사용 가능한지(ACTIVE) 확인합니다.
 * @param authCode 검증할 인증 코드 문자열.
 * @return 유효하고 사용 가능한 경우 해당 domain::PrePaymentCode 객체.
 * 그렇지 않은 경우 기본 생성된 PrePaymentCode 객체.
 */
domain::PrePaymentCode PrepaymentService::getPrepaymentDetailsIfActive(const std::string& authCode) {
    // UC14 (인증코드 유효성 검증)
    try {
        domain::PrePaymentCode prepayCode = prepayCodeRepository_.findByCode(authCode);

        if (prepayCode.getCode().empty()) { // findByCode가 못 찾으면 기본 객체 반환 가정
            // UC14 Exceptional E1: "시스템에 저장되 있지 않은 인증코드이면, 시스템은 에러메시지를 호출해 보여준다." [cite: 30]
            errorService_.processOccurredError(ErrorType::AUTH_CODE_NOT_FOUND, "인증 코드(" + authCode + ")를 찾을 수 없음");
            // 헤더 주석에는 CodeNotFoundException을 던진다고 되어 있으나, ErrorService 사용.
            return domain::PrePaymentCode(); // 기본 객체 반환
        }

        if (!prepayCode.isUsable()) { // isUsable()은 status가 ACTIVE인지 확인
            // UC14 Exceptional E1: "이미 사용된 인증코드이거나..." [cite: 30]
            errorService_.processOccurredError(ErrorType::AUTH_CODE_ALREADY_USED, "이미 사용된 인증 코드(" + authCode + ")");
            // 헤더 주석에는 CodeAlreadyUsedException을 던진다고 되어 있으나, ErrorService 사용.
            return domain::PrePaymentCode(); // 기본 객체 반환
        }
        // UC14 Typical Courses: "(S): 저장하고 있는 코드...와 입력 코드가 일치하는지 확인한다." -> 일치 및 ACTIVE 확인됨
        return prepayCode;
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "선결제 상세 정보 조회 중 오류: " + std::string(e.what()));
        return domain::PrePaymentCode(); // 기본 객체 반환
    }
}

/**
 * @brief 인증 코드의 상태를 "USED"로 변경합니다.
 * @param authCode 사용 완료 처리할 인증 코드 문자열.
 */
void PrepaymentService::changeAuthCodeStatusToUsed(const std::string& authCode) {
    // UC14 Typical Courses: "(S): 일치할 경우 AuthCode를 만료시킨다." [cite: 30] (만료를 USED 상태로 변경)
    try {
        bool success = prepayCodeRepository_.updateStatus(authCode, domain::CodeStatus::USED);
        if (!success) {
            // updateStatus가 false를 반환하는 경우는 코드가 없거나, 이미 USED 상태였거나, 기타 이유일 수 있음.
            // getPrepaymentDetailsIfActive에서 이미 ACTIVE 상태임을 확인했어야 함.
            // 여기서 false가 반환된다면 예기치 않은 상황일 수 있음.
            errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드(" + authCode + ") 상태 변경 실패(이미 사용되었거나 찾을 수 없음)");
            // 헤더 주석에는 CodeNotFoundException 또는 VendingMachineException을 던진다고 되어 있으나, ErrorService 사용.
        }
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "인증 코드 상태 변경 중 오류: " + std::string(e.what()));
    }
}

/**
 * @brief 다른 자판기로부터 수신한 선결제 요청 정보를 등록합니다.
 * @param certCode 수신한 인증 코드.
 * @param drinkCode 수신한 음료 코드.
 * @param vmidForOrder 이 선결제 건을 위해 생성될 Order에 기록될 자판기 ID (요청한 자판기 ID).
 */
void PrepaymentService::recordIncomingPrepayment(const std::string& certCode, const std::string& drinkCode, const std::string& vmidForOrder) {
    // UC15 (선결제요청 수신 및 재고 확보)
    // "(S): Drink.stock 값을 확인해 1이상이면 Drink.stock을 1감소시키고 PrePayment.code = AuthCode.code로 저장한다" [cite: 32]
    // 이 메소드는 PrePayment.code = AuthCode.code로 저장하는 부분에 해당. 재고 감소는 InventoryService 담당.
    try {
        // 1. Order 객체 생성 (이 선결제에 대한 기록용)
        //    vmidForOrder는 요청을 보낸 자판기(src_id), drinkCode, certCode를 사용.
        //    수량은 기본적으로 1, 결제 상태는 선결제가 이미 완료된 것으로 간주하여 "APPROVED" 또는 "PREPAID" 같은 특수 상태로 설정 가능.
        //    여기서는 "PENDING_DISPENSE" 와 같이 수령 대기 상태로 둘 수도 있음.
        //    일단은 간단히 APPROVED로 설정 (이 부분은 Order의 상태 관리 정책에 따라 달라질 수 있음)
        auto orderForPrepayment = std::make_shared<domain::Order>(vmidForOrder, drinkCode, 1, certCode, "APPROVED");
        // OrderRepository에 이 주문을 저장할 수도 있지만, PrepaymentCode가 Order를 참조하므로,
        // Order는 PrepaymentCode와 함께 관리되거나, PrepaymentCode가 필요한 Order 정보만 가질 수도 있음.
        // 현재 PrePaymentCode는 std::shared_ptr<domain::Order> heldOrder_attribute; 를 가짐.
        // 따라서 Order를 생성하고 저장한 후, PrePaymentCode를 저장해야 함.
        // 다만, 이 Order는 "다른 자판기에서 발생한 선결제에 대한 우리 자판기의 기록"이므로,
        // 우리 자판기의 OrderRepository에 저장하는 것이 맞는지, 아니면 PrepaymentService 내부에서만 관리되는
        // 임시 Order 객체인지 설계에 따라 달라짐.
        // 헤더에는 OrderRepository& orderRepo를 받고 있으므로, 여기에 저장하는 것이 자연스러움.
        orderRepository_.save(*orderForPrepayment);


        // 2. PrePaymentCode 객체 생성 및 저장
        domain::PrePaymentCode newPrepaymentCode(certCode, domain::CodeStatus::ACTIVE, orderForPrepayment);
        prepayCodeRepository_.save(newPrepaymentCode);

    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "수신된 선결제 정보 등록 중 오류: " + std::string(e.what()));
    }
}

/**
 * @brief 지정된 길이의 랜덤 영숫자 문자열을 생성합니다. (private helper)
 * @param length 생성할 문자열의 길이.
 * @return 생성된 랜덤 영숫자 문자열.
 */
std::string PrepaymentService::generateRandomAlphanumericString(size_t length) const {
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, charset.length() - 1);
    std::string random_string;
    random_string.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        random_string += charset[distribution(generator)];
    }
    return random_string;
}

} // namespace service