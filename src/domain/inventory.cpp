#include "domain/inventory.h"

namespace domain {
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


void inventory::reduceDrink() {
    if (qty_ > 0) {
        qty_--;
    }
}



int inventory :: getQty () const {
    return qty_;
}
}
