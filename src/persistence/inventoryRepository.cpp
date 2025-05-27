#include "../include/persistence/inventoryRepository.h"
#include <string>

using namespace domain;

namespace persistence {

std::vector<domain::inventory> inventoryRepository::allDrinks = {
    inventory(Drink("Pepsi", 1500, "D001"), 1),
    inventory(Drink("Sprite", 1500, "D002"), 1),
    inventory(Drink("Vita500", 1600, "D003"), 1),
    inventory(Drink("Powerade", 1600, "D004"), 1),
    inventory(Drink("Monster", 2500, "D005"), 1),
    inventory(Drink("Orange Juice", 2700, "D006"), 1),
    inventory(Drink("Gatorade", 1800, "D007"), 1),
    inventory(Drink("Corn Silk Tea", 1800, "D008"), 1),
    inventory(Drink("Bacchus", 1700, "D009"), 1),
    inventory(Drink("Vitamin Drink", 1000, "D010"), 1),
    inventory(Drink("Boseong Green Tea", 1500, "D011"), 1),
    inventory(Drink("Milkis", 1500, "D012"), 1),
    inventory(Drink("Cocopalm", 1600, "D013"), 1),
    inventory(Drink("Tropicana", 1600, "D014"), 1),
    inventory(Drink("Cafe Latte Can", 1200, "D015"), 1),
    inventory(Drink("Americano Can", 1200, "D016"), 1),
    inventory(Drink("Let's Be", 1300, "D017"), 1),
    inventory(Drink("Pocari Sweat", 1400, "D018"), 1),
    inventory(Drink("Fanta Orange", 1500, "D019"), 1),
    inventory(Drink("Cantata", 1600, "D020"), 1)
};


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
        List.emplace_back(inventory.getDrink().getName(), inventory.getDrink().getPrice());
    }
    return List;
}


//unit 테스트 기록 필요.
bool inventoryRepository::isValid(const std::string& drink) {
    for (const auto& inventory : allDrinks) {
        if (inventory.getDrink().getName() == drink) {
            return inventory.getQty() > 0;
        }
    }
    return false;  // 음료를 찾지 못한 경우
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