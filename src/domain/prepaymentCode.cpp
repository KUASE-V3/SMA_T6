#include "domain/prepaymentCode.h"
#include <cstdlib>
#include <string>
#include <memory>

namespace domain {

std::string PrePaymentCode::generateRandomCode() {
    static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result;
    for (int i = 0; i < 5; ++i) {
        int index = std::rand() % (sizeof(charset) - 1);
        result += charset[index];
    }
    return result;
}

// 나머지 멤버 함수는 헤더에서 inline으로 구현됨


} // namespace domain