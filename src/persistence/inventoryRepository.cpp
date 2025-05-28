#include "persistence/inventoryRepository.h"
#include "domain/inventory.h"
#include <stdexcept>

namespace persistence {


void InventoryRepository::addOrUpdateStock(const domain::Inventory& inventoryItem) {
    stock_[inventoryItem.getDrinkCode()] = inventoryItem;
}

bool InventoryRepository::isDrinkHandled(const std::string& drinkCode) const {
    return stock_.count(drinkCode) > 0;
}

bool InventoryRepository::hasStock(const std::string& drinkCode) const {
    auto it = stock_.find(drinkCode);
    if (it != stock_.end()) {
        return it->second.getQty() >= 1;
    }
    return false;
}

bool InventoryRepository::decreaseStockByOne(const std::string& drinkCode) {
    auto it = stock_.find(drinkCode);
    if (it != stock_.end()) {
        try {
            it->second.decreaseQuantity(1);
            return true;
        } catch (const std::out_of_range& /* e */) {
            return false;
        }
    }
    return false;
}

/**
 * @brief 특정 음료 코드에 해당하는 Inventory 객체를 반환합니다.
 */
domain::Inventory InventoryRepository::getInventoryByDrinkCode(const std::string& drinkCode) const {
    auto it = stock_.find(drinkCode);
    if (it != stock_.end()) {
        return it->second; // 찾았으면 해당 Inventory 객체 반환
    }
    // 찾지 못했으면, drinkCode가 비어있거나 qty가 0인 기본 Inventory 객체 반환
    return domain::Inventory();
}

} // namespace persistence