#include "inventory.h"

inventory::inventory(const Drink& drink, int qty)
    : drink_(drink), qty_(qty) {}


Drink inventory::getDrink() const {
return drink_;
}

bool inventory::isEmpty(){
    if (qty_ <= 0)  {
        return true;
    }
    
}


// qty_ 1 감소 (0 미만으로는 감소 안 함)
void inventory::reduceDrink() {
    if (qty_ > 0) {
        qty_--;
    }
}