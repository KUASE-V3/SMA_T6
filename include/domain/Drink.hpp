#pragma once
#include <string>
//�̰� �׳� �׽�Ʈ���� �����ڵ��Դϴ�. ������ �� �����ؾ��մϴ�.
namespace domain {

class Drink {
public:
    explicit Drink(std::string code = "00") : code_(std::move(code)) {}
    const std::string& code() const { return code_; }
private:
    std::string code_;
};

} // namespace domain
