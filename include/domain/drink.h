#ifndef DRINK_H
#define DRINK_H

#include <string>

namespace domain {
// 모든 음료의 가격은 1000원으로 통일
const int DEFAULT_DRINK_PRICE = 1000;

class Drink {
private:
    std::string drinkCode_attribute; // 음료 코드 (예: "01")
    std::string name_attribute;      // 음료 이름 (예: "콜라")
    int price_attribute;             // 음료 가격

public:
    Drink(const std::string& code = "", const std::string& name = "", int price = DEFAULT_DRINK_PRICE)
        : drinkCode_attribute(code), name_attribute(name), price_attribute(price) {}

    // Getters
    std::string getDrinkCode() const { return drinkCode_attribute; }
    std::string getName() const { return name_attribute; }
    int getPrice() const { return price_attribute; }


};
} // namespace domain

#endif // DRINK_H
