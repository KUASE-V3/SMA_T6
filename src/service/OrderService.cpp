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
    std::cerr << "orderservice.approve " << status << std::endl;
}

void OrderService::createOrder(const std::string& code) {
    drinkCode = code;
    std::cerr << "DrinkCode: " << drinkCode << std::endl;
}

void OrderService::attachPrePay(const std::string& code) {
    prepayCode = code;
    std::cerr << "Code: " << prepayCode << std::endl;
}
