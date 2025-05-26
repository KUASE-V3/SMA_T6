// PrepaymentService.cpp
#include "service/PrepaymentService.hpp"
#include "domain/prepaymentCode.h"
#include "persistence/prepayCodeRepository.h"

#include <iostream>

namespace service {
bool PrepaymentService::isValid(const std::string& code) {
    std::cerr << "[선결제 코드 유효성 검사] 입력된 코드: " << code << std::endl;
    return true;  // 임시 반환
}

std::string PrepaymentService::isSueCode() {
    domain::PrepaymentCode prepayCode;
    std::string code = prepayCode.generate();
    return code;

}


//UC 15 선결제 요청 수신 - 인증코드 저장하는 함수
void PrepaymentService::saveCode(const domain::Order& order) {
    domain::PrepaymentCode prepayCode;
    persistence::PrepaymentCodeRepository prepayCodeRepo;
    prepayCode.hold(order);
    prepayCodeRepo.save(prepayCode);
}
}