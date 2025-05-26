#ifndef ORDER_SERVICE_HPP
#define ORDER_SERVICE_HPP

#include <string>
#include "domain/order.h"

class OrderService {
public:
    OrderService(); // Order* order ?��?��

    void approve(const std::string& paymentID, bool success);
    domain::Order createOrder(const std::string& drinkCode, const std::string& prepayCode);

private:
    std::string status;
    std::string drinkCode;
    std::string prepayCode;
};

#endif
