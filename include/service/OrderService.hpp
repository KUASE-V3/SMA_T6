#ifndef ORDER_SERVICE_HPP
#define ORDER_SERVICE_HPP

#include <string>
#include "domain/order.h"

class OrderService {
public:
    OrderService(); // Order* order ?��?��

    void approve(const std::string& paymentID);
    domain::Order createOrder(const std::string& drinkID, const std::string& prepayCode);

private:
    std::string status;
    std::string drinkCode;
    std::string prepayCode;
};

#endif
