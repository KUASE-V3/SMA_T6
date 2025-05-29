#include "service/PrepaymentService.hpp"
#include "persistence/prepayCodeRepository.h"
#include "persistence/OrderRepository.hpp"
#include "domain/prepaymentCode.h"
#include "domain/order.h"
#include "service/ErrorService.hpp"

#include <random>
#include <string>
#include <algorithm> // std::all_of
#include <memory>    // std::make_shared
#include <stdexcept> // Repository에서 발생 가능한 예외 처리용

namespace service {
PrepaymentService::PrepaymentService(
    persistence::PrepayCodeRepository& prepayCodeRepo,
    persistence::OrderRepository& orderRepo,
    service::ErrorService& errorService
) : prepayCodeRepository_(prepayCodeRepo),
    orderRepository_(orderRepo),
    errorService_(errorService) {}

std::string PrepaymentService::generateAuthCodeString() {
    return generateRandomAlphanumericString(5);
}

void PrepaymentService::registerPrepayment(const std::string& authCode, std::shared_ptr<domain::Order> order) {
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

bool PrepaymentService::isValidAuthCodeFormat(const std::string& authCode) const {
    if (authCode.length() != 5) {
        return false;
    }
    return std::all_of(authCode.begin(), authCode.end(), ::isalnum);
}

domain::PrePaymentCode PrepaymentService::getPrepaymentDetailsIfActive(const std::string& authCode) {
    try {
        domain::PrePaymentCode prepayCode = prepayCodeRepository_.findByCode(authCode);

        if (prepayCode.getCode().empty()) {
            errorService_.processOccurredError(ErrorType::AUTH_CODE_NOT_FOUND, "인증 코드(" + authCode + ")를 찾을 수 없습니다.");
            return domain::PrePaymentCode();
        }

        if (!prepayCode.isUsable()) {
            errorService_.processOccurredError(ErrorType::AUTH_CODE_ALREADY_USED, "이미 사용된 인증 코드(" + authCode + ")입니다.");
            return domain::PrePaymentCode();
        }
        return prepayCode;
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "선결제 상세 정보 조회 중 시스템 오류: " + std::string(e.what()));
        return domain::PrePaymentCode();
    }
}

void PrepaymentService::changeAuthCodeStatusToUsed(const std::string& authCode) {
    try {
        bool success = prepayCodeRepository_.updateStatus(authCode, domain::CodeStatus::USED);
        if (!success) {
            errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드(" + authCode + ") 상태를 USED로 변경 실패 (찾을 수 없거나 이미 처리됨)");
        }
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "인증 코드 상태 변경 중 시스템 오류: " + std::string(e.what()));
    }
}

void PrepaymentService::recordIncomingPrepayment(const std::string& certCode, const std::string& drinkCode, const std::string& vmidForOrder) {
    try {
        auto orderForThisPrepayment = std::make_shared<domain::Order>(
            vmidForOrder,
            drinkCode,
            1,
            certCode,
            "APPROVED"
        );
        orderRepository_.save(*orderForThisPrepayment);

        domain::PrePaymentCode newPrepaymentCode(certCode, domain::CodeStatus::ACTIVE, orderForThisPrepayment);
        prepayCodeRepository_.save(newPrepaymentCode);

    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "수신된 선결제 정보 등록 중 시스템 오류: " + std::string(e.what()));
    }
}


/**
 * @brief 지정된 길이의 랜덤 영숫자 문자열을 생성하는 내부 헬퍼 함수입니다.
 * std::random_device를 사용하여 난수 생성의 비결정성을 높이려고 시도합니다.
 * @param length 생성할 문자열의 길이.
 * @return 생성된 랜덤 영숫자 문자열.
 */
std::string PrepaymentService::generateRandomAlphanumericString(size_t length) const {
    // PFR R.82: 인증코드는 알파벳 대/소문자와 숫자를 포함하는 랜덤한 5자리의 문자열
    static const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    // sizeof(charset)은 널 종료 문자까지 포함하므로, 실제 문자 집합의 크기는 -1,
    // 유효한 인덱스는 0부터 (실제 문자 집합 크기 - 1)이므로, max_index는 (sizeof(charset) - 2)가 됩니다.
    const size_t max_index = (sizeof(charset) - 2); //

    std::string random_string;
    random_string.reserve(length);

    std::random_device rd; 
    for (size_t i = 0; i < length; ++i) {
        
        std::mt19937 local_generator(rd()); 
        std::uniform_int_distribution<size_t> distribution(0, max_index);
        random_string += charset[distribution(local_generator)];
    }
    return random_string;
}

} // namespace service