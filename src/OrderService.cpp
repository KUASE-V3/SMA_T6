// OrderService.cpp
// ------------------------------
#include "OrderService.hpp"

OrderService::OrderService(Order* order) : order(order) {}

void OrderService::approve(const std::string& paymentID, bool success) {
    if (success) {
        order->setStatus("Approved");
    } else {
        order->setStatus("Declined");
    }
}

Order OrderService::createOrder(const std::string& drinkCode) {
    Order order;
    order.setDrinkCode(drinkCode);  // 예시
    return order;
}

Order OrderService::attachPrePay(const Order& order, const std::string& code) {
    return order.attachPrePay(code);  //  Order 객체의 함수 호출
}
