// #include <gtest/gtest.h>
// #include "../include/domain/inventory.h"
// #include "../include/domain/drink.h"

// TEST(InventoryTest, Constructor) {
//     Drink drink("μ½λΌ", 1500, "D001");
//     inventory inv(drink, 5);
//     EXPECT_EQ(inv.getDrink().getName(), "μ½λΌ");
//     EXPECT_EQ(inv.getDrink().getPrice(), 1500);
//     EXPECT_EQ(inv.getDrink().getCode(), "D001");
// }

// TEST(InventoryTest, GetDrink) {                     //?λ²€ν λ¦¬μ? ?λ§ν¬κ°?? Έ?€κΈ?
//     Drink drink("?¬?΄?€", 1200, "D002");
//     inventory inv(drink, 3);
//     Drink result = inv.getDrink();
//     EXPECT_EQ(result.getName(), "?¬?΄?€");
//     EXPECT_EQ(result.getPrice(), 1200);
//     EXPECT_EQ(result.getCode(), "D002");
// }

// TEST(InventoryTest, IsEmpty) {
//     Drink drink("????", 1300, "D003");
//     inventory inv1(drink, 0);                       //?¬κ³ κ?? ?? ? λ°ν κ°?
//     ASSERT_TRUE(inv1.isEmpty());

//     inventory inv2(drink, 2);                       //?¬κ³ κ?? ?? ? λ°ν κ°?
//     ASSERT_FALSE(inv2.isEmpty());                   
// }

// TEST(InventoryTest, ReduceDrink) {
//     Drink drink("λͺ¬μ€?°", 2000, "D004");
//     inventory inv(drink, 2);                        //?¬κ³ κ?? ?? ? ?¬κ³? -1
//     inv.reduceDrink();
//     ASSERT_FALSE(inv.isEmpty());
    
//     inv.reduceDrink();                          // ?¬κ³ κ?? ?? ? ?¬κ³? -1
//     ASSERT_TRUE(inv.isEmpty());
// }