#ifndef ORDER_H
#define ORDER_H

#include <string>
#include "drink.h"

class Order {
    public:
        Order();
        Order(const Drink& drink, const std::string& code, const std::string& payStatus);

        Order attachPrePay(const Drink& drink, const std::string& code);  
        void setStatus(const std::string& paystatus);      //"Declined", "Approved", "Pending"
        Drink getDrink() const;  // 음료 정보 반환
        std::string getOrderID() const;  // 주문 ID 반환
        std::string getPayStatus() const;  // 결제 상태 반환

    private:
            Drink drink_;
        std::string orderID_;
        std::string payStatus_;
};

#endif