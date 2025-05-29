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

// 랜덤 영숫자 문자열 생성
std::string PrepaymentService::generateRandomAlphanumericString(size_t length) const { //
    static const char charset[] = //
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 2); //

    std::string random_string; //
    random_string.reserve(length); //
    std::random_device rd; //
    for (size_t i = 0; i < length; ++i) { //
        std::mt19937 local_generator(rd()); //
        std::uniform_int_distribution<size_t> distribution(0, max_index); //
        random_string += charset[distribution(local_generator)]; //
    }
    return random_string; //
}

} // namespace service