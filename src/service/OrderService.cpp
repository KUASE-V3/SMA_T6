#include "service/OrderService.hpp"
#include <iostream>

using namespace domain;

OrderService::OrderService() {}

void OrderService::approve(Order& order, bool success) {
    if (success) {
        order.setStatus("Approved");
    } else {
        order.setStatus("Declined");
    }

    std::cerr << "[결제 상태 처리] certCode: " << order.certCode()
              << ", 상태: " << (success ? "Approved" : "Declined") << std::endl;
}

Order OrderService::createOrder(const Drink& drink) {
    Order order("T1", drink);  // 자판기 ID는 예시로 "T1"
    std::cerr << "[UC16] 주문 생성 - 음료: " << drink.getName() << std::endl;
    return order;
}