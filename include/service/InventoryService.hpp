// InventoryService.hpp
// ------------------------------
#ifndef INVENTORY_SERVICE_HPP
#define INVENTORY_SERVICE_HPP

#include <string>
#include <vector>
// #include "InventoryRepository.hpp"

class InventoryService {
public:
    std::vector<std::pair<std::string, int>> CallInventorySer();      // UC1
    bool getSaleValid(const std::string& drink);                      // UC3
    std::string reduceDrink(const std::string& drink);                // UC7
    bool validOVMStock(const std::string& drinkName);                 // UC17


};

#endif
