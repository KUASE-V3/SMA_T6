// PrepaymentService.hpp
// ------------------------------
#ifndef PREPAYMENT_SERVICE_HPP
#define PREPAYMENT_SERVICE_HPP

#include <string>

class PrepaymentService {
public:
    bool isValid(const std::string& code);      // UC13
    std::string issueCode();               // UC12
    void issueCode(Order& order);  // UC15
};

#endif
