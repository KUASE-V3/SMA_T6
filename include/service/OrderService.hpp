#ifndef ORDER_SERVICE_HPP
#define ORDER_SERVICE_HPP

#include <string>
#include "domain/order.h"
#include "domain/drink.h"

class OrderService {
public:
    OrderService();

    // UC5, UC6
    void approve(domain::Order& order, bool success);

    // UC16
    domain::Order createOrder(const domain::Drink& drink);  // O
};

#endif
