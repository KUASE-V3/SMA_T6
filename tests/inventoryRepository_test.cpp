#include <gtest/gtest.h>
#include "../include/persistence/inventoryRepository.h"
#include "../include/domain/drink.h"
#include "../include/domain/inventory.h"


using domain::Drink;
using domain::inventory;
using persistence::inventoryRepository;

TEST(InventoryRepositoryTest, SetAndGetAllDrinks) {
    std::vector<Drink> testDrinks = {
        Drink("콜라", 1500, "D001"),
        Drink("?��?��?��", 1500, "D002"),
        Drink("?��??? ?��?���?", 1600, "D003"),
        Drink("?��??? ?��?��", 1600, "D004"),
        Drink("몬스?��", 2500, "D005"),
        Drink("?��?���?", 2700, "D006"),
        Drink("게토?��?��", 1800, "D007"),
        Drink("?��?��?��?��?��", 1800, "D008"),
        Drink("?��?���?", 1700, "D009"),
        Drink("비�??500", 1000, "D010"),
        Drink("보성?���?", 1500, "D011"),
        Drink("?��?��?��?��?���?", 1500, "D012"),
        Drink("코코?��", 1600, "D013"),
        Drink("?��로피카나", 1600, "D014"),
        Drink("?��?��?��", 2000, "D015"),
        Drink("?��메리카노 캔커?��", 1200, "D016"),
        Drink("조�???�� ?��리�???��", 1300, "D017"),
        Drink("TOP ?��?��?�� ?��메리카노", 1400, "D018"),
        Drink("�??��?��", 1500, "D019"),
        Drink("칸�?????", 1600, "D020")
    };

    //?��벤토�? 리스?�� ?��?��
    std::vector<inventory> inventoryList;
    const int INITIAL_QUANTITY = 10;  // 초기 ?���? ?��?��

    for (const auto& drink : testDrinks) {
        inventoryList.push_back(inventory(drink, INITIAL_QUANTITY));
    }

    EXPECT_EQ(inventoryList.size(), 20);

    EXPECT_EQ(inventoryList[0].getDrink().getName(), "콜라");
    EXPECT_EQ(inventoryList[0].getDrink().getPrice(), 1500);
    EXPECT_EQ(inventoryList[0].getDrink().getCode(), "D001");
    EXPECT_EQ(inventoryList[0].getQty(), 10);

    

    //?��?���??���? 객체 ?��?�� ?�� ?��?��?�� ?��벤토�?(?���? + ?��?��) ?��?�� ????��
    inventoryRepository repo;
    repo.setAllDrinks(inventoryList);

    //????��?�� 리스?�� 반환
    auto resultList = repo.getList();
    EXPECT_EQ(resultList.size(), 20);

    // �? 번째 ?���? �?�?
    EXPECT_EQ(resultList[0].first, "콜라");
    EXPECT_EQ(resultList[0].second, 10);

    // 마�??�? ?���? �?�?
    EXPECT_EQ(resultList[19].first, "칸�?????");
    EXPECT_EQ(resultList[19].second, 10);

    // 중간 ?���? �?�?
    EXPECT_EQ(resultList[9].first, "비�??500");
    EXPECT_EQ(resultList[9].second, 10);

}

