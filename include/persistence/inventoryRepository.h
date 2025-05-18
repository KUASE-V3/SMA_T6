#ifndef INVENTORYREPOSITORY_H
#define INVENTORYREPOSITORY_H

#include <string>

#include "../domain/drink.h"
#include "../domain/inventory.h"
#include <vector>

namespace persistence {
class inventoryRepository {

    public:
        void setAllDrinks(const std::vector<domain::inventory>& drinks);
        static const std::vector<domain::inventory>& getAllDrinks();;

        std::vector<std :: pair<std::string, int>> getList();

        

    private:
        static std::vector<domain::inventory> allDrinks;

        
};
}
#endif
 
/*
 Drink("콜라", 1.50, "D001"),
    Drink("사이다", 1.50, "D002"),
    Drink("환타 오렌지", 1.60, "D003"),
    Drink("환타 포도", 1.60, "D004"),
    Drink("몬스터", 2.50, "D005"),
    Drink("레드불", 2.70, "D006"),
    Drink("게토레이", 1.80, "D007"),
    Drink("파워에이드", 1.80, "D008"),
    Drink("이프로", 1.70, "D009"),
    Drink("비타500", 1.00, "D010"),
    Drink("보성녹차", 1.50, "D011"),
    Drink("옥수수수염차", 1.50, "D012"),
    Drink("코코팜", 1.60, "D013"),
    Drink("트로피카나", 1.60, "D014"),
    Drink("핫식스", 2.00, "D015"),
    Drink("아메리카노 캔커피", 1.20, "D016"),
    Drink("조지아 오리지널", 1.30, "D017"),
    Drink("TOP 스위트 아메리카노", 1.40, "D018"),
    Drink("밀키스", 1.50, "D019"),
    Drink("칸타타", 1.60, "D020")
*/