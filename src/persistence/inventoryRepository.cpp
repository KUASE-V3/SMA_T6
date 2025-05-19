#include "../include/persistence/inventoryRepository.h"
#include <string>

using namespace domain;

namespace persistence {
std::vector<domain::inventory> inventoryRepository::allDrinks = {
    inventory(Drink("펩시", 1500, "D001"), 10),
    inventory(Drink("스프라이트", 1500, "D002"), 10),
    inventory(Drink("비타500", 1600, "D003"), 10),
    inventory(Drink("파워에이드", 1600, "D004"), 10),
    inventory(Drink("몬스터", 2500, "D005"), 10),
    inventory(Drink("오렌지주스", 2700, "D006"), 10),
    inventory(Drink("게토레이", 1800, "D007"), 10),
    inventory(Drink("옥수수 수염차", 1800, "D008"), 10),
    inventory(Drink("박카스", 1700, "D009"), 10),
    inventory(Drink("비타민음료", 1000, "D010"), 10),
    inventory(Drink("보성녹차", 1500, "D011"), 10),
    inventory(Drink("밀키스", 1500, "D012"), 10),
    inventory(Drink("코코팜", 1600, "D013"), 10),
    inventory(Drink("트로피카나", 1600, "D014"), 10),
    inventory(Drink("카페라떼 캔커피", 1200, "D015"), 10),
    inventory(Drink("아메리카노 캔커피", 1200, "D016"), 10),
    inventory(Drink("레쓰비", 1300, "D017"), 10),
    inventory(Drink("포카리스웨트", 1400, "D018"), 10),
    inventory(Drink("환타 오렌지", 1500, "D019"), 10),
    inventory(Drink("칸타타", 1600, "D020"), 10)
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