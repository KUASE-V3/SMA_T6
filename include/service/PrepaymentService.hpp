#pragma once
#include <string>

namespace service {
class PrepaymentService {
public:
    bool isValid(const std::string& code);
    std::string isSueCode();  // ��Ÿ����: issueCode �� �ƴ϶� isSueCode �� ������
    void saveCode(const domain::Order& order);
    
};
}
