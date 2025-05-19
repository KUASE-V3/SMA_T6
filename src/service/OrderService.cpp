// OrderService.cpp
// ------------------------------
#include "service/OrderService.hpp"
#include <iostream>

OrderService::OrderService() {}

void OrderService::approve(const std::string& paymentID, bool success) {
    if (success) {
        status = "Approved";
    } else {
        status = "Declined";
    }
    std::cerr << "[?��?�� ?��?��] " << status << std::endl;
}

void OrderService::createOrder(const std::string& code) {
    drinkCode = code;
    std::cerr << "[주문 ?��?��] DrinkCode: " << drinkCode << std::endl;
}

void OrderService::attachPrePay(const std::string& code) {
    prepayCode = code;
    std::cerr << "[?��결제 코드 ?���?] Code: " << prepayCode << std::endl;
}

