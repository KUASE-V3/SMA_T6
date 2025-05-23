#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include "domain/drink.h"

namespace domain {

class inventory {
    public:
        inventory();
        inventory(const Drink& drink, int qty);

        Drink getDrink() const;
        bool isEmpty() const;
        void reduceDrink(const Drink& drink);

        int getQty() const;

    private:
        Drink drink_;
        int qty_;
};

}

#endif
