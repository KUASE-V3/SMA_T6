#pragma once
#include <string>
//이거 그냥 테스트위한 샘플코드입니다. 병합할 때 삭제해야합니다.
namespace domain {

class Drink {
public:
    explicit Drink(std::string code = "00") : code_(std::move(code)) {}
    const std::string& code() const { return code_; }
private:
    std::string code_;
};

} // namespace domain
