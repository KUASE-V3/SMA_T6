// OrderService.cpp
// ------------------------------
#include "OrderService.hpp"
#include <iostream>

OrderService::OrderService() {}

void OrderService::approve(const std::string& paymentID, bool success) {
    if (success) {
        status = "Approved";
    } else {
        status = "Declined";
    }
    std::cerr << "[승인 상태] " << status << std::endl;
}

void OrderService::createOrder(const std::string& code) {
    drinkCode = code;
    std::cerr << "[주문 생성] DrinkCode: " << drinkCode << std::endl;
}

void OrderService::attachPrePay(const std::string& code) {
    prepayCode = code;
    std::cerr << "[선결제 코드 연결] Code: " << prepayCode << std::endl;
}
