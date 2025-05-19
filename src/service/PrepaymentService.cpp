#include "service/PrepaymentService.hpp"
#include "persistence/prepayCodeRepository.h"
#include <iostream>

using namespace domain;
using namespace service;
using namespace persistence;

// UC12: 단순 인증코드 생성
PrepaymentCode PrepaymentService::isSueCode() {
    PrepaymentCode code;
    std::string generated = code.generate();

    std::cerr << "[UC12] 인증코드 발급됨: " << generated << std::endl;
    return code;
}

// UC15: 주문을 보관하고 저장소에 저장
PrepaymentCode PrepaymentService::isSueCode(const Order& order) {
    PrepaymentCode code;
    std::string generated = code.generate();

    std::cerr << "[UC15] 인증코드 발급 및 주문 보관: " << generated << std::endl;

    PrepaymentCode held = code.hold(order);     // 주문 객체 연결
    PrepaymentCodeRepository repo;
    repo.save(held);  // 인스턴스 생성 후 save() 호출


    return held;
}

// UC13: 인증코드 유효성 검사
bool PrepaymentService::isValid(const std::string& code) {
    return PrepaymentCode::isUsable(code);  // 정적 검사
}

// UC14: 인증코드 상태 변경
void PrepaymentService::changeStatusCode(const std::string& code) {
    //TOOD 상태 변경 메서드 구현 필요
    
}