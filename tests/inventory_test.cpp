#include <gtest/gtest.h>
#include "../include/domain/inventory.h"
#include "../include/domain/drink.h"

TEST(InventoryTest, Constructor) {
    Drink drink("콜라", 1500, "D001");
    inventory inv(drink, 5);
    EXPECT_EQ(inv.getDrink().getName(), "콜라");
    EXPECT_EQ(inv.getDrink().getPrice(), 1500);
    EXPECT_EQ(inv.getDrink().getCode(), "D001");
}

TEST(InventoryTest, GetDrink) {                     //안벤토리에서 드링크가져오기
    Drink drink("사이다", 1200, "D002");
    inventory inv(drink, 3);
    Drink result = inv.getDrink();
    EXPECT_EQ(result.getName(), "사이다");
    EXPECT_EQ(result.getPrice(), 1200);
    EXPECT_EQ(result.getCode(), "D002");
}

TEST(InventoryTest, IsEmpty) {
    Drink drink("환타", 1300, "D003");
    inventory inv1(drink, 0);                       //재고가 없을 때 반환 값
    ASSERT_TRUE(inv1.isEmpty());

    inventory inv2(drink, 2);                       //재고가 있을 때 반환 값
    ASSERT_FALSE(inv2.isEmpty());                   
}

TEST(InventoryTest, ReduceDrink) {
    Drink drink("몬스터", 2000, "D004");
    inventory inv(drink, 2);                        //재고가 있을 때 재고 -1
    inv.reduceDrink();
    ASSERT_FALSE(inv.isEmpty());
    
    inv.reduceDrink();                          // 재고가 없을 때 재고 -1
    ASSERT_TRUE(inv.isEmpty());
}