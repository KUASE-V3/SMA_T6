// PrepaymentService.cpp
#include "service/PrepaymentService.hpp"
#include "domain/prepaymentCode.h"
#include "persistence/prepayCodeRepository.h"

#include <iostream>

namespace service {
bool PrepaymentService::isValid(const std::string& code) {              //UC13 인증코드 입력 처리
    return domain::PrepaymentCode::isUsable(code);
}

std::string PrepaymentService::isSueCode() {
    domain::PrepaymentCode prepayCode;
    std::string code = prepayCode.generate();
    return code;

}


//UC 15 선결제 요청 수신 - 인증코드 저장하는 함수
void PrepaymentService::saveCode(const domain::Order& order) {
    domain::PrepaymentCode prepayCode;
    prepayCode = prepayCode.hold(order);
    persistence::PrepaymentCodeRepository::save(prepayCode);
}
}