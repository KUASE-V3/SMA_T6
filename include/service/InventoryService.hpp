#pragma once
#include <string>
#include <vector>

namespace service {
class InventoryService {
public:
    std::vector<std::pair<std::string, int>> CallInventorySer();//uc1
    bool getSaleValid(const std::string& drink);//uc3
    std::string reduceDrink(const std::string& drink);//uc7
    bool validOVMStock(const std::string& drinkName);//uc17
};
}
