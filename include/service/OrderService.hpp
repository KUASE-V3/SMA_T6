#ifndef ORDER_SERVICE_HPP
#define ORDER_SERVICE_HPP

#include <string>

class OrderService {
public:
    OrderService(); // Order* order 삭제

    void approve(const std::string& paymentID, bool success);
    void createOrder(const std::string& drinkCode);  // 반환형 변경
    void attachPrePay(const std::string& prepayCode); // 파라미터 변경

private:
    std::string status;
    std::string drinkCode;
    std::string prepayCode;
};

#endif
