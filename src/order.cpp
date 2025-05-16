#include "order.h"



Order Order::attachPrePay(const Drink& drink, const std::string& code) {
    return Order(drink, code, "Pending");  // 생성 시 기본 상태는 Pending
}

void Order::setStatus(const std::string& status) {
    if (status == "Approved" || status == "Declined") {
        payStatus_ = status;
    }
}