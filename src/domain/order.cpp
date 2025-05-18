#include "../include/domain/order.h"

Order::Order()
: drink_(), orderID_(""), payStatus_("Pending"){}

Order::Order(const Drink& drink, const std::string& code, const std::string& payStatus)
: drink_(drink), orderID_(code), payStatus_(payStatus) {}

Order Order::attachPrePay(const Drink& drink, const std::string& code) {
    return Order(drink, code, "Pending");  // 생성 시 기본 상태는 Pending
}

void Order::setStatus(const std::string& status) {
    if (status == "Approved" || status == "Declined") {
        payStatus_ = status;
    }
}

Drink Order::getDrink() const {
    return drink_;
}

std::string Order::getOrderID() const {
    return orderID_;
}

std::string Order::getPayStatus() const {
    return payStatus_;
}