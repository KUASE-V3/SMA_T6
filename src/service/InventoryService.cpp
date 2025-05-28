#include "service/InventoryService.hpp"
#include "persistence/inventoryRepository.h"
#include "persistence/DrinkRepository.hpp"
#include "domain/drink.h" // domain::Drink 사용을 위해 필요

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

/**
 * @brief 시스템에 정의된 모든 음료 종류의 목록을 반환합니다. (UC1. 음료 목록 조회 및 표시)
 * @return domain::Drink 객체들의 벡터.
 */
std::vector<domain::Drink> InventoryService::getAllDrinkTypes() {
    try {
        // (R1.1. 전체 20종류의 음료 메뉴를 사용자에게 표시 [cite: 65])
        return drinkRepository_.findAll();
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "전체 음료 목록 조회 중 오류: " + std::string(e.what()));
        return {};
    }
}

/**
 * @brief 특정 음료의 현재 자판기 내 판매 가능 여부와 가격 정보를 확인합니다. (UC3. 현재 자판기 재고 확인)
 * @param drinkCode 확인할 음료의 코드.
 * @return DrinkAvailabilityInfo 구조체.
 */
InventoryService::DrinkAvailabilityInfo InventoryService::checkDrinkAvailabilityAndPrice(const std::string& drinkCode) {
    DrinkAvailabilityInfo info;

    try {
        domain::Drink drink = drinkRepository_.findByDrinkCode(drinkCode);
        if (drink.getDrinkCode().empty()) {
            errorService_.processOccurredError(ErrorType::DRINK_NOT_FOUND, "음료 코드(" + drinkCode + ")에 해당하는 음료 없음");
            return info; // isAvailable = false, price = 0 상태로 반환
        }
        info.price = drink.getPrice();

        // (R1.3. 현재 자판기의 재고를 확인한다. [cite: 65])
        bool isHandled = inventoryRepository_.isDrinkHandled(drinkCode);

        if (isHandled) {
            // UC3. Typical Courses of Events - 1. "(S): 사용자가 선택한 음료에 대해 시스템은 재고를 조회해 재고 유무를 판단한다."
            // hasStock은 재고가 1 이상인지 확인합니다.
            if (inventoryRepository_.hasStock(drinkCode)) {
                info.isAvailable = true;
            } else {
                info.isAvailable = false; // 취급은 하나 재고 없음
            }
        } else {
            info.isAvailable = false; // 취급 안 함
        }

    } catch (const std::exception& e) {
        // UC3. Exceptional Courses of Events - E1: "재고 정보를 불러오지 못했거나 데이터가 손상된 경우, 시스템은 에러메시지를 호출해 보여준다." [cite: 8]
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "음료(" + drinkCode + ") 재고 확인 중 오류: " + std::string(e.what()));
    }

    return info;
}

/**
 * @brief 특정 음료의 재고를 1 감소시킵니다. (UC7. 음료 배출 제어 - 일반결제 시)
 * @param drinkCode 재고를 감소시킬 음료의 코드.
 */
void InventoryService::decreaseStock(const std::string& drinkCode) {
    // UC7. Typical Courses of Events - 2. "(S): 일반결제인 경우 재고를 1감소시다" [cite: 16]
    try {
        bool success = inventoryRepository_.decreaseStockByOne(drinkCode);
        if (!success) {
            errorService_.processOccurredError(ErrorType::DRINK_OUT_OF_STOCK, "음료(" + drinkCode + ") 재고 차감 실패(부족 또는 없음)");
        }
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "음료(" + drinkCode + ") 재고 차감 중 오류: " + std::string(e.what()));
    }
}

}  // namespace service