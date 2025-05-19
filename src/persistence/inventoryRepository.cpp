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
 

std::vector<std::pair<std::string, int>> inventoryRepository::getList() {
    std::vector<std::pair<std::string, int>> List;
    for (const auto& inventory : allDrinks) {
        List.emplace_back(inventory.getDrink().getName(), inventory.getQty());
    }
    return List;
}
}