#pragma once

#include <string>
#include <vector>
#include <utility> // for std::pair

// 레포지토리 및 서비스 클래스 전방 선언
namespace domain { // domain::Drink 전방 선언 추가
    class Drink;
}
namespace persistence {
    class InventoryRepository;
    class DrinkRepository;
}
namespace service {
    class ErrorService;
}

namespace service {

class InventoryService {
public:
    InventoryService(
        persistence::InventoryRepository& inventoryRepo,
        persistence::DrinkRepository& drinkRepo,
        service::ErrorService& errorService
    );

    // 시스템에 정의된 모든 음료 종류의 목록을 반환 (UC1. 음료 목록 조회 및 표시)
    std::vector<domain::Drink> getAllDrinkTypes();

    // 특정 음료의 판매 가능 여부와 가격 정보를 반환 (UC3. 현재 자판기 재고 확인)
    struct DrinkAvailabilityInfo {
        bool isAvailable = false;   // 현재 자판기에서 구매 가능한지 (취급하며 재고도 있는지)
        int price = 0;              // 음료 가격
    };
    DrinkAvailabilityInfo checkDrinkAvailabilityAndPrice(const std::string& drinkCode);

    // 특정 음료의 재고를 1 감소 (UC7. 음료 배출 제어 - 일반결제인 경우)
    void decreaseStock(const std::string& drinkCode);

private:
    persistence::InventoryRepository& inventoryRepository_;
    persistence::DrinkRepository& drinkRepository_;
    service::ErrorService& errorService_;
};

} // namespace service