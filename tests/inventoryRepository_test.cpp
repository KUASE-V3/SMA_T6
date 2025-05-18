#include <gtest/gtest.h>
#include "../include/persistence/inventoryRepository.h"
#include "../include/domain/drink.h"
#include "../include/domain/inventory.h"

TEST(InventoryRepositoryTest, SetAndGetAllDrinks) {
    std::vector<Drink> testDrinks = {
        Drink("콜라", 1500, "D001"),
        Drink("사이다", 1500, "D002"),
        Drink("환타 오렌지", 1600, "D003"),
        Drink("환타 포도", 1600, "D004"),
        Drink("몬스터", 2500, "D005"),
        Drink("레드불", 2700, "D006"),
        Drink("게토레이", 1800, "D007"),
        Drink("파워에이드", 1800, "D008"),
        Drink("이프로", 1700, "D009"),
        Drink("비타500", 1000, "D010"),
        Drink("보성녹차", 1500, "D011"),
        Drink("옥수수수염차", 1500, "D012"),
        Drink("코코팜", 1600, "D013"),
        Drink("트로피카나", 1600, "D014"),
        Drink("핫식스", 2000, "D015"),
        Drink("아메리카노 캔커피", 1200, "D016"),
        Drink("조지아 오리지널", 1300, "D017"),
        Drink("TOP 스위트 아메리카노", 1400, "D018"),
        Drink("밀키스", 1500, "D019"),
        Drink("칸타타", 1600, "D020")
    };

    //인벤토리 리스트 생성
    std::vector<inventory> inventoryList;
    const int INITIAL_QUANTITY = 10;  // 초기 재고 수량

    for (const auto& drink : testDrinks) {
        inventoryList.push_back(inventory(drink, INITIAL_QUANTITY));
    }

    EXPECT_EQ(inventoryList.size(), 20);

    EXPECT_EQ(inventoryList[0].getDrink().getName(), "콜라");
    EXPECT_EQ(inventoryList[0].getDrink().getPrice(), 1500);
    EXPECT_EQ(inventoryList[0].getDrink().getCode(), "D001");
    EXPECT_EQ(inventoryList[0].getQty(), 10);

    

    //레퍼지토리 객체 생성 후 정했던 인벤토리(음료 + 수량) 내용 저장
    inventoryRepository repo;
    repo.setAllDrinks(inventoryList);

    //저장된 리스트 반환
    auto resultList = repo.getList();
    EXPECT_EQ(resultList.size(), 20);

    // 첫 번째 음료 검증
    EXPECT_EQ(resultList[0].first, "콜라");
    EXPECT_EQ(resultList[0].second, 10);

    // 마지막 음료 검증
    EXPECT_EQ(resultList[19].first, "칸타타");
    EXPECT_EQ(resultList[19].second, 10);

    // 중간 음료 검증
    EXPECT_EQ(resultList[9].first, "비타500");
    EXPECT_EQ(resultList[9].second, 10);

}

