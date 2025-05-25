#include "domain/inventory.h"
#include "domain/drink.h"


using namespace domain;

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



void inventory::reduceDrink(const Drink& selecteddrink) {
    if (selecteddrink.getName() == drink_.getName() && qty_ > 0) {
        qty_--;
    }
}



int inventory::getQty() const {
    return qty_;
}

