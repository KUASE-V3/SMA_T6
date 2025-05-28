#include "service/InventoryService.hpp"
#include "persistence/inventoryRepository.h" 
#include "persistence/DrinkRepository.hpp"   // hpp 사용 일관성
#include "domain/drink.h"                   
#include "domain/inventory.h"                
#include "service/ErrorService.hpp"

#include <vector>
#include <string>

namespace service {

InventoryService::InventoryService(
    persistence::InventoryRepository& inventoryRepo,
    persistence::DrinkRepository& drinkRepo,
    service::ErrorService& errorService
) : inventoryRepository_(inventoryRepo),
    drinkRepository_(drinkRepo),
    errorService_(errorService) {}

std::vector<domain::Drink> InventoryService::getAllDrinkTypes() {
    try {
        return drinkRepository_.findAll(); // [cite: 65]
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "전체 음료 목록 조회 중 오류: " + std::string(e.what()));
        return {};
    }
}

InventoryService::DrinkAvailabilityInfo InventoryService::checkDrinkAvailabilityAndPrice(const std::string& drinkCode) {
    DrinkAvailabilityInfo info; // isAvailable = false, price = 0, currentStock = 0 으로 초기화

    try {
        domain::Drink drink = drinkRepository_.findByDrinkCode(drinkCode);
        if (drink.getDrinkCode().empty()) { // findByDrinkCode가 못 찾으면 기본 Drink 객체 반환 가정
            errorService_.processOccurredError(ErrorType::DRINK_NOT_FOUND, "음료 코드(" + drinkCode + ")에 해당하는 음료 없음");
            return info;
        }
        info.price = drink.getPrice();

        bool isHandled = inventoryRepository_.isDrinkHandled(drinkCode); // [cite: 65]

        if (isHandled) {
            domain::Inventory invItem = inventoryRepository_.getInventoryByDrinkCode(drinkCode);
            // getInventoryByDrinkCode가 못 찾거나 해당 음료가 없으면 기본 Inventory 객체 (qty 0) 반환
            info.currentStock = invItem.getQty();

            if (info.currentStock > 0) { // inventoryRepository_.hasStock(drinkCode) 와 동일한 조건
                info.isAvailable = true;
            } else {
                info.isAvailable = false; // 취급은 하나 재고 없음
            }
        } else {
            info.isAvailable = false; // 취급 안 함
            info.currentStock = 0;  // 재고도 당연히 없음
        }

    } catch (const std::exception& e) {
        // [cite: 8]
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "음료(" + drinkCode + ") 재고 확인 중 오류: " + std::string(e.what()));
    }

    return info;
}

void InventoryService::decreaseStock(const std::string& drinkCode) {
    try {
        bool success = inventoryRepository_.decreaseStockByOne(drinkCode); // [cite: 16]
        if (!success) {
            errorService_.processOccurredError(ErrorType::DRINK_OUT_OF_STOCK, "음료(" + drinkCode + ") 재고 차감 실패(부족 또는 없음)");
        }
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "음료(" + drinkCode + ") 재고 차감 중 오류: " + std::string(e.what()));
    }
}

}  // namespace service