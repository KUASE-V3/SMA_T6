#include "../include/persistence/inventoryRepository.h"
#include <string>

using namespace domain;

namespace persistence {
std::vector<domain::inventory> inventoryRepository::allDrinks;

void inventoryRepository::setAllDrinks(const std::vector<domain::inventory>& drinks) {
    allDrinks = drinks;
}

const std::vector<domain::inventory>& inventoryRepository::getAllDrinks() {
    return allDrinks;
}
 

//uc1 ? getList메소?
std::vector<std::pair<std::string, int>> inventoryRepository::getList() {
    std::vector<std::pair<std::string, int>> List;
    for (const auto& inventory : allDrinks) {
        List.emplace_back(inventory.getDrink().getName(), inventory.getQty());
    }
    return List;
}


//unit 테스트 기록 필요.
bool inventoryRepository::isValid(const domain::Drink& drink) {
    for (const auto& inventory : allDrinks) {
        if (inventory.getDrink().getName() == drink.getName()) {
            if (inventory.getQty() > 0)    return true;
            else    return false;
        }
    }

}

void inventoryRepository::changeQty(const domain::Drink& drink) {
    for (auto& inventory : allDrinks) {
        if (inventory.getDrink().getName() == drink.getName()) {
            inventory.reduceDrink(drink);
            break;
        }
    }
}

bool inventoryRepository::isEmptyRepo(const domain::Drink& drink) const {
    for (const auto& inventory : allDrinks) {
        if (inventory.getDrink().getName() == drink.getName()) {
            return inventory.isEmpty();
        }
    }
    return true;  // 음료를 찾지 못한 경우도 true 반환
}
}