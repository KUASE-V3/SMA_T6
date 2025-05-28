#pragma once

#include <string>
#include <vector>
#include <utility>

namespace domain {
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

    std::vector<domain::Drink> getAllDrinkTypes();

    struct DrinkAvailabilityInfo {
        bool isAvailable = false;
        int price = 0;
        int currentStock = 0; // 현재 자판기의 해당 음료 실제 재고량
    };
    DrinkAvailabilityInfo checkDrinkAvailabilityAndPrice(const std::string& drinkCode);

    void decreaseStock(const std::string& drinkCode);

private:
    persistence::InventoryRepository& inventoryRepository_;
    persistence::DrinkRepository& drinkRepository_;
    service::ErrorService& errorService_;
};

} // namespace service