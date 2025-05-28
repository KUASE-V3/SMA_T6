#pragma once

#include <string>
#include <map> // 재고 저장을 위해 사용

#include "domain/inventory.h" // domain::Inventory 사용
namespace persistence {

class InventoryRepository {
public:
    InventoryRepository() = default;

    // 1. 초기 재고 설정 (또는 기존 재고에 아이템 추가/업데이트)
    void addOrUpdateStock(const domain::Inventory& inventoryItem);

    // 2. 특정 음료가 현재 자판기에서 취급(판매)하는 음료인지 확인
    bool isDrinkHandled(const std::string& drinkCode) const;

    // 3. 특정 음료의 재고가 1 이상인지 확인 (취급하는 음료 대상)
    bool hasStock(const std::string& drinkCode) const;

    // 4. 특정 음료의 재고를 1 감소 (판매 완료 또는 선결제 재고 확보 시)
    //    성공하면 true, 실패(해당 음료 없음, 재고 부족 등) 시 false.
    bool decreaseStockByOne(const std::string& drinkCode);

private:
    // 메모리 내 재고 저장소 (자판기당 최대 7종류 음료)
    // std::string은 drinkCode
    std::map<std::string, domain::Inventory> stock_;
};

} // namespace persistence