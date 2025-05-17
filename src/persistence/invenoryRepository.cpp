#include "inventoryRepository.h"
#include <string>

std::vector<Drink> inventoryRepository::allDrinks;


void inventoryRepository::setAllDrinks(const std::vector<Drink>& drinks) {
    allDrinks = drinks;
}

const std::vector<Drink>& inventoryRepository::getAllDrinks() {
    return allDrinks;
}


//uc1 번 getList메소드
std::vector<std::pair<std::string, int>> inventoryRepository::getList() {
    std::vector<std::pair<std::string, double>> List;
    for (const auto& drink : allDrinks) {
        List.emplace_back(drink.getName(), drink.getPrice());
    }
}