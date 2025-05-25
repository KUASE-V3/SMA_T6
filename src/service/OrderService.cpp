// OrderService.cpp
// ------------------------------
#include "service/OrderService.hpp"
#include "domain/order.h"
#include <iostream>

OrderService::OrderService() {}

void OrderService::approve(const std::string& paymentID) {                  //개선 필요
      
}

domain::Order OrderService::createOrder(const std::string& drinkID, const std::string& precode) {
    

    domain::Order order; 
    order = order.attachPrePay(drinkID, precode);       //드링크 id와 인증코드를 붙인다 -> 선택한 drink의 drinkID는 어떻게 가져올것인가?

    return order; 
}

