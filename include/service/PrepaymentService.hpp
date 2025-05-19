#pragma once
#include <string>

namespace service {
class PrepaymentService {
public:
    bool isValid(const std::string& code);
    std::string isSueCode();  // 오타주의: issueCode 가 아니라 isSueCode 로 돼있음
    void saveCode(const domain::Order& order);
    
};
}
