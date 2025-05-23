#pragma once
#include <string>

namespace service {
class PrepaymentService {
public:
    bool isValid(const std::string& code);
    void isSueCode(); 
};
}
