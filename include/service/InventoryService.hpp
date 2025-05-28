#pragma once

#include <string>
#include <vector>


#include "domain/drink.h"
#include "domain/inventory.h"

// 레포지토리 및 서비스 클래스 전방 선언 (헤더 순환 참조 방지 및 컴파일 시간 단축)
// .cpp 파일에서는 실제 헤더를 include 합니다.
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

    std::vector<domain::Drink> getAllDrinkTypes();
    std::vector<domain::Inventory> getCurrentVmFullStock();

    struct DrinkAvailabilityInfo {
        bool isAvailable = false;
        int price = 0;
        int currentStock = 0;
    };
    DrinkAvailabilityInfo checkDrinkAvailabilityAndPrice(const std::string& drinkCode, int quantityNeeded = 1);

    void decreaseStock(const std::string& drinkCode, int quantity);

private:
    // 멤버 변수도 참조 타입으로 변경
    persistence::InventoryRepository& inventoryRepository_;
    persistence::DrinkRepository& drinkRepository_;
    service::ErrorService& errorService_;
};

} // namespace service