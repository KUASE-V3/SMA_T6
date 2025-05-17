// PrepaymentService.hpp
#ifndef PREPAYMENT_SERVICE_HPP
#define PREPAYMENT_SERVICE_HPP

#include <string>

class PrepaymentService {
public:
    bool isValid(const std::string& code);
    void isSueCode();  // Order 없앰

    // void issueCode(Order& order);  // 주석 처리 또는 제거
};

#endif
