#ifndef ORDER_SERVICE_HPP
#define ORDER_SERVICE_HPP

#include <string>

class OrderService {
public:
    OrderService(); // Order* order ?��?��

    void approve(const std::string& paymentID, bool success);
    void createOrder(const std::string& drinkCode);  // 반환?�� �?�?
    void attachPrePay(const std::string& prepayCode); // ?��?��미터 �?�?
    

private:
    std::string status;
    std::string drinkCode;
    std::string prepayCode;
};

#endif
