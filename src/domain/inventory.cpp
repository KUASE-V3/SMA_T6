#include "inventory.h"

namespace domain {
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


// qty_ 1 ê°ì†Œ (0 ë¯¸ë§Œ?œ¼ë¡œëŠ” ê°ì†Œ ?•ˆ ?•¨)
void inventory::reduceDrink() {
    if (qty_ > 0) {
        qty_--;
    }
}
}