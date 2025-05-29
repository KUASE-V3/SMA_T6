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

/**
 * @brief 시스템에 정의된 모든 음료 종류의 목록을 반환합니다.
 * (UC1. 음료 목록 조회 및 표시 - Typical Course 1: "(S): 자판기가 Drink List를를 확인해...")
 * @return domain::Drink 객체들의 벡터. Repository 접근 실패 시 빈 벡터 반환 및 오류 처리.
 */
std::vector<domain::Drink> InventoryService::getAllDrinkTypes() {
    try {
        // PFR - R1.1: 전체 20종류의 음료 메뉴를 사용자에게 표시
        return drinkRepository_.findAll();
    } catch (const std::exception& e) {
        // UC1 E1: "Drink List를 정상적으로 불러올 수 없을 때는 에러메시지를 호출해 보여준다."
        // ErrorService를 통해 오류 처리 (예: REPOSITORY_ACCESS_ERROR)
        errorService_.processOccurredError(
            ErrorType::REPOSITORY_ACCESS_ERROR,
            "전체 음료 목록 조회 중 오류 발생: " + std::string(e.what())
        );
        return {}; // 빈 벡터 반환
    }
}

/**
 * @brief 특정 음료의 현재 자판기 내 판매 가능 여부, 가격, 실제 재고량을 확인합니다.
 * (UC2 Typical Course 2: "(S): 시스템은 Inventory를 참조해 해당 음료가 자판기에서 판매 가능한지 확인한다.")
 * (UC3 Typical Course 1: "(S): 사용자가 선택한 음료에 대해 시스템은 재고를 조회해 재고 유무를 판단한다.")
 * @param drinkCode 확인할 음료의 코드.
 * @return DrinkAvailabilityInfo 구조체 (isAvailable, price, currentStock 포함).
 */
InventoryService::DrinkAvailabilityInfo InventoryService::checkDrinkAvailabilityAndPrice(const std::string& drinkCode) {
    DrinkAvailabilityInfo info; // isAvailable = false, price = 0, currentStock = 0 으로 자동 초기화

    try {
        // 1. 음료 기본 정보(이름, 가격) 조회 (DrinkRepository 사용)
        domain::Drink drink = drinkRepository_.findByDrinkCode(drinkCode);
        if (drink.getDrinkCode().empty()) { // findByDrinkCode가 못 찾으면 기본 Drink 객체 반환
            // UC1 E1 또는 UC3에서 음료 정보를 못 찾는 경우
            errorService_.processOccurredError(ErrorType::DRINK_NOT_FOUND, "음료 코드(" + drinkCode + ")에 해당하는 음료 정보 없음");
            return info; // isAvailable = false
        }
        info.price = drink.getPrice();

        // 2. 현재 자판기에서 해당 음료를 취급하는지 및 실제 재고량 확인 (InventoryRepository 사용)
        // PFR R1.3: 현재 자판기의 재고를 확인한다.
        if (inventoryRepository_.isDrinkHandled(drinkCode)) {
            domain::Inventory invItem = inventoryRepository_.getInventoryByDrinkCode(drinkCode);
            info.currentStock = invItem.getQty(); // 실제 재고량 설정

            if (info.currentStock > 0) {
                info.isAvailable = true; // 취급하고 재고도 있음 -> 구매 가능
            } else {
                info.isAvailable = false; // 취급은 하나 현재 재고 없음
            }
        } else {
            info.isAvailable = false; // 현재 자판기에서 취급하지 않는 음료
            info.currentStock = 0;
        }
    } catch (const std::exception& e) {
        // UC3 E1: "재고 정보를 불러오지 못했거나 데이터가 손상된 경우, 시스템은 에러메시지를 호출해 보여준다."
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "음료(" + drinkCode + ") 정보/재고 확인 중 오류: " + std::string(e.what()));
        // info는 기본값(isAvailable=false, currentStock=0)으로 반환됨
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


/**
 * @brief 특정 음료의 재고를 지정된 수량만큼 감소시킵니다.
 * @param drinkCode 재고를 감소시킬 음료의 코드.
 * @param amount 감소시킬 수량.
 */
void InventoryService::decreaseStockByAmount(const std::string& drinkCode, int amount) {
    try {
        bool success = inventoryRepository_.decreaseStockByAmount(drinkCode, amount);
        if (!success) {
            // 재고 부족 또는 해당 음료 없음
            errorService_.processOccurredError(ErrorType::INSUFFICIENT_STOCK_FOR_DECREASE, // 또는 DRINK_OUT_OF_STOCK
                                           "음료(" + drinkCode + ") " + std::to_string(amount) + "개 재고 차감 실패");
        }
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR,
                                           "음료(" + drinkCode + ") 재고 차감 중 오류: " + std::string(e.what()));
    }
}

}  // namespace service