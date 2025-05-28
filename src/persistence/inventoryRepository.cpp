#include "persistence/inventoryRepository.h" // InventoryRepository 헤더 파일 포함
#include "domain/inventory.h" // domain::Inventory 사용
#include <stdexcept>      // std::out_of_range 등 예외 사용 가능성 (Inventory 내부에서)

namespace persistence {

// InventoryRepository::InventoryRepository() {} // 기본 생성자

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
    return false; // 취급하지 않는 음료는 재고 없음
}

bool InventoryRepository::decreaseStockByOne(const std::string& drinkCode) {
    auto it = stock_.find(drinkCode);
    if (it != stock_.end()) {
        // it->second는 domain::Inventory 객체입니다.
        // domain::Inventory의 decreaseQuantity는 재고 부족 시 예외를 발생시킵니다.
        try {
            it->second.decreaseQuantity(1); // 재고 1 감소 시도
            return true; // 성공
        } catch (const std::out_of_range& /* e */) { // 예외 객체 e는 사용하지 않음
            // domain::Inventory에서 재고 부족으로 예외 발생
            // 서비스 계층으로 예외를 전파하지 않고 false를 반환하여 실패를 알림.
            return false;
        }
    }
    return false; // 해당 음료를 찾을 수 없음 (취급 안 함)
}

} // namespace persistence