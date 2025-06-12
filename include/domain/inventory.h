#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <stdexcept> // 나중에 정의 에러로 변환 예정 

namespace domain {
class Inventory {
private:
    std::string drinkCode_attribute;
    int qty_attribute;

public:
    Inventory(const std::string& dCode = "", int qty = 0)
        : drinkCode_attribute(dCode), qty_attribute(qty) {
        if (qty < 0) { // 초기 수량이 음수일 경우 방지
            this->qty_attribute = 0;
        }
        if (qty > 99) { // 최대 재고 99개 제한
            this->qty_attribute = 99;
        }
    }

    std::string getDrinkCode() const { return drinkCode_attribute; }
    int getQty() const { return qty_attribute; }

    bool isEmpty() const {
        return qty_attribute <= 0;
    }

    void decreaseQuantity(int amount) {
        if (amount <= 0) return; // 감소량이 0 이하이면 변경 없음
        if (qty_attribute < amount) {
            // 예외 처리: 현재 재고보다 많은 양을 감소시키려 할 때
            throw std::out_of_range("Cannot decrease quantity below zero. Current: " +
                                    std::to_string(qty_attribute) + ", Tried to decrease by: " + std::to_string(amount));
        }
        qty_attribute -= amount;
    }

};
}

#endif // INVENTORY_H