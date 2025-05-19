#include "service/InventoryService.hpp"

#include "persistence/inventoryRepository.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace domain;
using namespace persistence;

namespace service {

std::vector<std::pair<std::string, int>> InventoryService::CallInventorySer() {
    inventoryRepository repo;
    return repo.getList();
}

bool InventoryService::getSaleValid(const std::string& drinkCode) {
    auto drinks = inventoryRepository::getAllDrinks();
    for (const auto& inv : drinks) {
        if (inv.getDrink().getCode() == drinkCode && inv.getQty() > 0)
            return true;
    }
    return false;
}

// UC7 다이어그램 흐름 그대로: 수량 감소 고려 없이 음료 반환만
Drink InventoryService::ReqReduceDrink(const std::string& drinkCode) {
    auto drinks = inventoryRepository::getAllDrinks();
    for (const auto& inv : drinks) {
        if (inv.getDrink().getCode() == drinkCode) {
            return inv.getDrink();  // 다이어그램상 reduce 후 반환된 drink
        }
    }
    throw std::runtime_error("해당 코드에 해당하는 음료 없음");
}

bool InventoryService::validOVMStock(const std::string& drinkName) {
    return true;

}

}
