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

    private:
            Drink drink_;
        std::string orderID_;
        std::string payStatus_;
};

#endif