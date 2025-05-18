// #include <gtest/gtest.h>
// #include "../include/domain/inventory.h"
// #include "../include/domain/drink.h"

// TEST(InventoryTest, Constructor) {
//     Drink drink("콜라", 1500, "D001");
//     inventory inv(drink, 5);
//     EXPECT_EQ(inv.getDrink().getName(), "콜라");
//     EXPECT_EQ(inv.getDrink().getPrice(), 1500);
//     EXPECT_EQ(inv.getDrink().getCode(), "D001");
// }

// TEST(InventoryTest, GetDrink) {                     //?��벤토리에?�� ?��링크�??��?���?
//     Drink drink("?��?��?��", 1200, "D002");
//     inventory inv(drink, 3);
//     Drink result = inv.getDrink();
//     EXPECT_EQ(result.getName(), "?��?��?��");
//     EXPECT_EQ(result.getPrice(), 1200);
//     EXPECT_EQ(result.getCode(), "D002");
// }

// TEST(InventoryTest, IsEmpty) {
//     Drink drink("?��???", 1300, "D003");
//     inventory inv1(drink, 0);                       //?��고�?? ?��?�� ?�� 반환 �?
//     ASSERT_TRUE(inv1.isEmpty());

//     inventory inv2(drink, 2);                       //?��고�?? ?��?�� ?�� 반환 �?
//     ASSERT_FALSE(inv2.isEmpty());                   
// }

// TEST(InventoryTest, ReduceDrink) {
//     Drink drink("몬스?��", 2000, "D004");
//     inventory inv(drink, 2);                        //?��고�?? ?��?�� ?�� ?���? -1
//     inv.reduceDrink();
//     ASSERT_FALSE(inv.isEmpty());
    
//     inv.reduceDrink();                          // ?��고�?? ?��?�� ?�� ?���? -1
//     ASSERT_TRUE(inv.isEmpty());
// }