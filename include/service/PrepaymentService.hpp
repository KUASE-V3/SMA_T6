#pragma once
#include <string>
#include <domain/order.h>

namespace service {
class PrepaymentService {
public:
    bool isValid(const std::string& code);
    std::string isSueCode(); 
    void saveCode(const domain::Order& order);
    
};
}
