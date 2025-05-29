// src/service/PrepaymentService.cpp

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

std::string PrepaymentService::generateAuthCodeString() { //
    return generateRandomAlphanumericString(5); //
}


// 인증 코드 형식 검사 (변경 없음)
bool PrepaymentService::isValidAuthCodeFormat(const std::string& authCode) const { //
    if (authCode.length() != 5) { //
        return false;
    }
    return std::all_of(authCode.begin(), authCode.end(), ::isalnum); //
}

// 인증 코드 유효성 및 사용 가능 여부 확인 
domain::PrePaymentCode PrepaymentService::getPrepaymentDetailsIfActive(const std::string& authCode) { //
    try {
        domain::PrePaymentCode prepayCode = prepayCodeRepository_.findByCode(authCode); //

        if (prepayCode.getCode().empty()) { //
            errorService_.processOccurredError(ErrorType::AUTH_CODE_NOT_FOUND, "인증 코드(" + authCode + ")를 찾을 수 없습니다."); //
            return domain::PrePaymentCode(); //
        }

        if (!prepayCode.isUsable()) { // ACTIVE 상태가 아님 (예: 이미 사용됨)
            errorService_.processOccurredError(ErrorType::AUTH_CODE_ALREADY_USED, "이미 사용되었거나 유효하지 않은 인증 코드(" + authCode + ")입니다."); //
            return domain::PrePaymentCode(); //
        }

        return prepayCode; //
    } catch (const std::exception& e) { //
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "선결제 상세 정보 조회 중 시스템 오류: " + std::string(e.what())); //
        return domain::PrePaymentCode(); //
    }
}

// 인증 코드 상태 'USED'로 변경 
void PrepaymentService::changeAuthCodeStatusToUsed(const std::string& authCode) { //
    try {
        bool success = prepayCodeRepository_.updateStatus(authCode, domain::CodeStatus::USED); //
        if (!success) { //
            errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "인증 코드(" + authCode + ") 상태를 USED로 변경 실패 (찾을 수 없거나 이미 처리됨)"); //
        }
    } catch (const std::exception& e) { //
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "인증 코드 상태 변경 중 시스템 오류: " + std::string(e.what())); //
    }
}

// 다른 자판기로부터 수신한 선결제 요청 기록 
void PrepaymentService::recordIncomingPrepayment(const std::string& certCode, const std::string& drinkCode, const std::string& vmidForOrder) { //
    try {
        auto orderForThisPrepayment = std::make_shared<domain::Order>( //
            vmidForOrder,       //
            drinkCode,          //
            1,                  //
            certCode,           //
            "APPROVED"          //
        );
        orderRepository_.save(*orderForThisPrepayment); //

        // 수신된 certCode를 현재 자판기의 PrepayCodeRepository에 저장 (처리해야 할 코드로 등록)
        domain::PrePaymentCode newPrepaymentCode(certCode, domain::CodeStatus::ACTIVE, orderForThisPrepayment); //
        prepayCodeRepository_.save(newPrepaymentCode); //

    } catch (const std::exception& e) { //
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "수신된 선결제 정보 등록 중 시스템 오류: " + std::string(e.what())); //
    }
}


    /**
     * @brief 지정된 길이의 랜덤 영숫자 문자열을 생성하는 내부 헬퍼 함수입니다.
     * std::random_device를 분포에 직접 사용하여 난수 생성의 예측 가능성을 낮추려고 시도합니다.
     * @param length 생성할 문자열의 길이.
     * @return 생성된 랜덤 영숫자 문자열.
     */
    std::string PrepaymentService::generateRandomAlphanumericString(size_t length) const {
        static const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        const size_t charset_size = sizeof(charset) - 2; // 널 종료 문자 제외한 실제 문자 수

        std::string result_string;
        result_string.reserve(length);

        std::random_device rd; // 비결정적 난수 생성기 시도
        // std::random_device 자체가 Non-deterministic Random Bit Generator(NRBG)로 사용될 수 있음
        // 이를 직접 std::uniform_int_distribution의 입력으로 사용합니다.
        // 이는 std::mt19937과 같은 PRNG를 사용하는 것보다 예측이 더 어렵게 만듭니다.

        std::uniform_int_distribution<size_t> distribution(0, charset_size);

        for (size_t i = 0; i < length; ++i) {
            result_string += charset[distribution(rd)]; // rd를 직접 난수 엔진처럼 사용
        }
        return result_string;
    }
} // namespace service