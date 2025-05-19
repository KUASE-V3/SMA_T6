#pragma once
#include <string>
#include <vector>
#include "domain/drink.h"

namespace service {
class InventoryService {
public:
    std::vector<std::pair<std::string, int>> CallInventorySer();      
    bool getSaleValid(const std::string& drinkCode);                  
    domain::Drink ReqReduceDrink(const std::string& drinkCode);       
    bool validOVMStock(const std::string& drinkName);                 

};
}