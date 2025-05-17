// OrderService.hpp
// ------------------------------
#ifndef ORDER_SERVICE_HPP
#define ORDER_SERVICE_HPP

#include <string>
#include "Order.hpp" 

class OrderService {
public:
    OrderService(Order* order);
    void approve(const std::string& paymentID, bool success);
    Order createOrder(const std::string& drinkCode); 
    Order attachPrePay(const Order& order, const std::string& prepayCode);

    
private:
    Order* order;
};
#endif
