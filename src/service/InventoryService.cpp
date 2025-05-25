// InventoryService.cpp
#include "service/InventoryService.hpp"
#include "../include/persistence/inventoryRepository.h"
#include <vector>
#include <string>
namespace service {
std::vector<std::pair<std::string, int>> InventoryService::CallInventorySer() {
    persistence::inventoryRepository repo;
    return repo.getList();
}

bool InventoryService::getSaleValid(const std::string& drinkname) {
    bool result = persistence::inventoryRepository::isValid(drinkname);
    return result;  
}

std::string InventoryService::ReqreduceDrink(const std::string& drink) {
    return drink;  // ?��?�� 반환
}

bool InventoryService::validOVMStock(const std::string& drinkName) {
    return true;  // ?��?�� 반환
}


}  // namespace service