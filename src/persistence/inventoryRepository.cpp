#include "persistence/inventoryRepository.h"
#include <string>

using namespace persistence;

std::vector<domain::Drink> inventoryRepository::all_;



void inventoryRepository::setAllDrinks(const std::vector<domain::Drink>& list) {
    all_ = list;
}

const std::vector<domain::Drink>& inventoryRepository::getAllDrinks() {
    return all_;
}