// ------------------------------
// InventoryService.cpp
// ------------------------------
#include "InventoryService.hpp"

std::vector<std::pair<std::string, int>> InventoryService::CallInventorySer() {
    return repository.getList();
}

bool InventoryService::getSaleValid(const std::string& drink) {
    return repository.isValid(drink);
}

std::string InventoryService::reduceDrink(const std::string& drink) {
    repository.changeQty(drink);
    return drink;
}

bool InventoryService::validOVMStock(const std::string& drinkName) {
    // 예시: DrinkRepository 또는 Inventory가 있으면 내부에서 직접 재고 확인
    for (const auto& drink : drinkList) {
        if (drink.getName() == drinkName && drink.getStock() > 0) {
            return true;
        }
    }
    return false;
}
