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


// ìž…ë ¥ë°›ì€ ìŒë£Œê°€ ì €ìž¥ëœ ìŒë£Œì™€ ê°™ì„ ë•Œë§Œ ìˆ˜ëŸ‰ ê°ì†Œ
void inventory::reduceDrink(const Drink& selecteddrink) {
    if (selecteddrink.getName() == drink_.getName() && qty_ > 0) {

        qty_--;
    }
}



int inventory :: getQty () const {
    return qty_;
}
}
