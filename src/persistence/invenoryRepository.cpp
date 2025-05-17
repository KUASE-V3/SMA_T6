#include "inventoryRepository.h"
#include <string>

std::vector<Drink> inventoryRepository::allDrinks;


void inventoryRepository::setAllDrinks(const std::vector<Drink>& drinks) {
    allDrinks = drinks;
}

const std::vector<Drink>& inventoryRepository::getAllDrinks() {
    return allDrinks;
}