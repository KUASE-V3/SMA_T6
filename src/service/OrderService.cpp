// OrderService.cpp
// ------------------------------
#include "service/OrderService.hpp"
#include "domain/order.h"
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

domain::Order OrderService::createOrder(const std::string& drinkcode, const std::string& precode) {
    domain::Drink drink("",0,drinkcode);
    domain::Order order("T5", drink, 1, "Pending");

    order.attachPrePay(precode);

    return order; 
}

