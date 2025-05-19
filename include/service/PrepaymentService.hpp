#pragma once
#include <string>
#include "domain/order.h"
#include "domain/prepaymentCode.h"

namespace service {
class PrepaymentService {
public:
    // UC12: 인증코드 발급 (단순 코드 생성)
    domain::PrepaymentCode isSueCode();

    // UC15: 인증코드 발급 + 주문 보관 + 저장
    domain::PrepaymentCode isSueCode(const domain::Order& order);

    // UC13: 인증코드 유효성 검사
    bool isValid(const std::string& code);
};
}