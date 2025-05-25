#pragma once
#include <vector>
#include <string>
#include <utility>
#include "domain/drink.h"
#include "domain/inventory.h"

namespace persistence {
    class inventoryRepository {
    private:
        static std::vector<domain::inventory> allDrinks;

    public:
        void setAllDrinks(const std::vector<domain::inventory>& drinks);
        const std::vector<domain::inventory>& getAllDrinks();
        std::vector<std::pair<std::string, int>> getList();
        static bool isValid(const std::string& drink);
        void changeQty(const domain::Drink& drink);
        bool isEmptyRepo(const domain::Drink& drink) const;
    };
} 