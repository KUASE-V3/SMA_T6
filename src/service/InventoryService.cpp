// InventoryService.cpp
#include "service/InventoryService.hpp"
#include <vector>
#include <string>
namespace service {
std::vector<std::pair<std::string, int>> InventoryService::CallInventorySer() {
    return {};  
}

bool InventoryService::getSaleValid(const std::string& drink) {
    return true; 
}

std::string InventoryService::reduceDrink(const std::string& drink) {
    return drink;
}

bool InventoryService::validOVMStock(const std::string& drinkName) {
    return true;  
}


}  // namespace service