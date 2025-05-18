
#include "../include/domain/inventory.h"

inventory::inventory(const Drink& drink, int qty)
    : drink_(drink), qty_(qty) {}


Drink inventory::getDrink() const {
    return drink_;
}


bool inventory::isEmpty() const {
    if (qty_ <= 0)  {
        return true;
    }

    else return false;
    
}



// 입력받은 음료가 저장된 음료와 같을 때만 수량 감소
void inventory::reduceDrink(const Drink& selecteddrink) {
    if (selecteddrink.getName() == drink_.getName() && qty_ > 0) {
        qty_--;
    }
}



int inventory :: getQty () const {
    return qty_;
}

