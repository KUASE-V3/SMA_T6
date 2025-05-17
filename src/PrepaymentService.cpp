// PrepaymentService.cpp
// ------------------------------
#include "PrepaymentService.hpp"

bool PrepaymentService::isValid(const std::string& code) {
    PrepaymentCode p;
    return p.isUsable(code);  
}

std::string PrepaymentService::isSueCode() {
    PrepaymentCode p;
    return p.rand();  // PrepaymentCode 객체로부터 인증코드 발급
}

void PrepaymentService::isSueCode(const Order& order) {
    PrepaymentCode p;
    p.hold(order);    // 인증코드에 order를 홀딩하고
    p.save(order);    // PrepaymentCodeRepository에 저장
}

