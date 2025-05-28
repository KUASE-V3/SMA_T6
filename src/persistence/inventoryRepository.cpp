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

/**
 * @brief 특정 음료의 재고를 지정된 수량만큼 감소시킵니다.
 * @param drinkCode 재고를 감소시킬 음료의 코드.
 * @param amount 감소시킬 수량.
 * @return 성공 시 true, 실패(해당 음료 없음, 재고 부족 등) 시 false.
 */
bool InventoryRepository::decreaseStockByAmount(const std::string& drinkCode, int amount) {
    if (amount <= 0) { // 감소량이 0 이하면 처리 안 함
        return false;
    }
    auto it = stock_.find(drinkCode);
    if (it != stock_.end()) {
        try {
            it->second.decreaseQuantity(amount); // domain::Inventory의 메소드 사용
            return true; // 성공
        } catch (const std::out_of_range&) {
            // domain::Inventory에서 재고 부족으로 예외 발생
            return false; // 실패 알림
        }
    }
    return false; // 해당 음료를 찾을 수 없음
}


} // namespace persistence