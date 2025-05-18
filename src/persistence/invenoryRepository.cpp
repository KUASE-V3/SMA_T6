#include "../include/persistence/inventoryRepository.h"
#include <string>

std::vector<inventory> inventoryRepository::allDrinks;


void inventoryRepository::setAllDrinks(const std::vector<inventory>& drinks) {
    allDrinks = drinks;
}

const std::vector<inventory>& inventoryRepository::getAllDrinks() {
    return allDrinks;
}
 

//uc1 번 getList메소드
std::vector<std::pair<std::string, int>> inventoryRepository::getList() {
    std::vector<std::pair<std::string, int>> List;
    for (const auto& inventory : allDrinks) {
        List.emplace_back(inventory.getDrink().getName(), inventory.getQty());
    }
    return List;
}