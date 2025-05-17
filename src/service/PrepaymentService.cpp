// PrepaymentService.cpp
#include "service/PrepaymentService.hpp"
#include <iostream>

namespace service {
bool PrepaymentService::isValid(const std::string& code) {
    std::cerr << "[선결제 코드 유효성 검사] 입력된 코드: " << code << std::endl;
    return true;  // 임시 반환
}

void PrepaymentService::isSueCode() {
    std::cerr << "[선결제 코드 발급] 실제 로직 없음 (임시 구현)" << std::endl;
}

// void PrepaymentService::issueCode(Order& order) {
//     // Order 관련 로직 제거 또는 나중 구현
// }
}
