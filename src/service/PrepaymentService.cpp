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
#include <stdexcept> 
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


bool PrepaymentService::isValidAuthCodeFormat(const std::string& authCode) const { //
    if (authCode.length() != 5) { //
        return false;
    }
    return std::all_of(authCode.begin(), authCode.end(), ::isalnum); //
}

domain::PrePaymentCode PrepaymentService::getPrepaymentDetailsIfActive(const std::string& authCode) { //
    try {
        domain::PrePaymentCode prepayCode = prepayCodeRepository_.findByCode(authCode); //

        if (prepayCode.getCode().empty()) { //
            errorService_.processOccurredError(ErrorType::AUTH_CODE_NOT_FOUND, "�씤利� 肄붾뱶(" + authCode + ")瑜� 李얠쓣 �닔 �뾾�뒿�땲�떎."); //
            return domain::PrePaymentCode(); //
        }

        if (!prepayCode.isUsable()) { // ACTIVE �긽�깭媛� �븘�떂 (�삁: �씠誘� �궗�슜�맖)
            errorService_.processOccurredError(ErrorType::AUTH_CODE_ALREADY_USED, "�씠誘� �궗�슜�릺�뿀嫄곕굹 �쑀�슚�븯吏� �븡��� �씤利� 肄붾뱶(" + authCode + ")�엯�땲�떎."); //
            return domain::PrePaymentCode(); //
        }

        return prepayCode; //
    } catch (const std::exception& e) { //
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "�꽑寃곗젣 �긽�꽭 �젙蹂� 議고쉶 以� �떆�뒪�뀥 �삤瑜�: " + std::string(e.what())); //
        return domain::PrePaymentCode(); //
    }
}

// �씤利� 肄붾뱶 �긽�깭 'USED'濡� 蹂�寃� 
void PrepaymentService::changeAuthCodeStatusToUsed(const std::string& authCode) { //
    try {
        bool success = prepayCodeRepository_.updateStatus(authCode, domain::CodeStatus::USED); //
        if (!success) { //
            errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "�씤利� 肄붾뱶(" + authCode + ") �긽�깭瑜� USED濡� 蹂�寃� �떎�뙣 (李얠쓣 �닔 �뾾嫄곕굹 �씠誘� 泥섎━�맖)"); //
        }
    } catch (const std::exception& e) { //
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "�씤利� 肄붾뱶 �긽�깭 蹂�寃� 以� �떆�뒪�뀥 �삤瑜�: " + std::string(e.what())); //
    }
}

// �떎瑜� �옄�뙋湲곕줈遺��꽣 �닔�떊�븳 �꽑寃곗젣 �슂泥� 湲곕줉 
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
        domain::PrePaymentCode newPrepaymentCode(certCode, domain::CodeStatus::ACTIVE, orderForThisPrepayment); //
        prepayCodeRepository_.save(newPrepaymentCode); //

    } catch (const std::exception& e) { //
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "�닔�떊�맂 �꽑寃곗젣 �젙蹂� �벑濡� 以� �떆�뒪�뀥 �삤瑜�: " + std::string(e.what())); //
    }
}


    std::string PrepaymentService::generateRandomAlphanumericString(size_t length) const {
        static const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        const size_t charset_size = sizeof(charset) - 2; 

        std::string result_string;
        result_string.reserve(length);

        std::random_device rd;
        std::uniform_int_distribution<size_t> distribution(0, charset_size);

        for (size_t i = 0; i < length; ++i) {
            result_string += charset[distribution(rd)]; 
        }
        return result_string;
    }
} // namespace service