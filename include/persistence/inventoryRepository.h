#pragma once

#include <string>
#include <map>
#include <optional> // std::optional 사용 가능

#include "domain/inventory.h"

namespace persistence {

class InventoryRepository {
public:
    InventoryRepository() = default;

    void addOrUpdateStock(const domain::Inventory& inventoryItem);
    bool isDrinkHandled(const std::string& drinkCode) const;
    bool hasStock(const std::string& drinkCode) const; // 재고가 1개 이상인지
    bool decreaseStockByOne(const std::string& drinkCode);

    /**
     * @brief 특정 음료 코드에 해당하는 Inventory 객체를 반환합니다.
     * 해당 음료를 취급하지 않거나 Inventory 객체를 찾을 수 없는 경우,
     * qty가 0이거나 drinkCode가 비어있는 기본 Inventory 객체를 반환할 수 있습니다.
     * 또는 std::optional<domain::Inventory>를 사용할 수도 있습니다.
     * 여기서는 domain::Inventory를 직접 반환하고, 호출부에서 유효성을 확인합니다.
     * @param drinkCode 조회할 음료 코드.
     * @return 해당 음료의 Inventory 객체. 찾지 못하면 기본 생성된 Inventory 객체.
     */
    domain::Inventory getInventoryByDrinkCode(const std::string& drinkCode) const;

private:
    std::map<std::string, domain::Inventory> stock_;
};

} // namespace persistence