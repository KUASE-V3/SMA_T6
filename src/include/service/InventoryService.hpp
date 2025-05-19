#pragma once
#include <string>
#include <vector>
#include <utility>
#include "domain/drink.h"

namespace service {
    class InventoryService {
    public:
        static bool getSaleValid(const std::string& drinkname);
        static std::vector<std::pair<std::string, int>> CallInventorySer();
        static std::string ReqReduceDrink(const std::string& drink);
        static bool validOVMStock(const std::string& drinkName);
    };
} 