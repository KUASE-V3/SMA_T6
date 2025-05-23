#include <gtest/gtest.h>
#include "../include/persistence/inventoryRepository.h"
#include "../include/domain/drink.h"
#include "../include/domain/inventory.h"


using domain::Drink;
using domain::inventory;
using persistence::inventoryRepository;

TEST(InventoryRepositoryTest, SetAndGetAllDrinks) {
    std::vector<Drink> testDrinks = {
        Drink("μ½λΌ", 1500, "D001"),
        Drink("?¬?΄?€", 1500, "D002"),
        Drink("???? ?€? μ§?", 1600, "D003"),
        Drink("???? ?¬?", 1600, "D004"),
        Drink("λͺ¬μ€?°", 2500, "D005"),
        Drink("? ?λΆ?", 2700, "D006"),
        Drink("κ²ν ? ?΄", 1800, "D007"),
        Drink("????΄?", 1800, "D008"),
        Drink("?΄?λ‘?", 1700, "D009"),
        Drink("λΉν??500", 1000, "D010"),
        Drink("λ³΄μ±?Ήμ°?", 1500, "D011"),
        Drink("?₯????Όμ°?", 1500, "D012"),
        Drink("μ½μ½?", 1600, "D013"),
        Drink("?Έλ‘νΌμΉ΄λ", 1600, "D014"),
        Drink("?«??€", 2000, "D015"),
        Drink("?λ©λ¦¬μΉ΄λΈ μΊμ»€?Ό", 1200, "D016"),
        Drink("μ‘°μ??? ?€λ¦¬μ???", 1300, "D017"),
        Drink("TOP ?€??Έ ?λ©λ¦¬μΉ΄λΈ", 1400, "D018"),
        Drink("λ°??€?€", 1500, "D019"),
        Drink("μΉΈν?????", 1600, "D020")
    };

    //?Έλ²€ν λ¦? λ¦¬μ€?Έ ??±
    std::vector<inventory> inventoryList;
    const int INITIAL_QUANTITY = 10;  // μ΄κΈ° ?¬κ³? ??

    for (const auto& drink : testDrinks) {
        inventoryList.push_back(inventory(drink, INITIAL_QUANTITY));
    }

    EXPECT_EQ(inventoryList.size(), 20);

    EXPECT_EQ(inventoryList[0].getDrink().getName(), "μ½λΌ");
    EXPECT_EQ(inventoryList[0].getDrink().getPrice(), 1500);
    EXPECT_EQ(inventoryList[0].getDrink().getCode(), "D001");
    EXPECT_EQ(inventoryList[0].getQty(), 10);

    

    //? ?Όμ§?? λ¦? κ°μ²΄ ??± ? ? ?? ?Έλ²€ν λ¦?(?λ£? + ??) ?΄?© ????₯
    inventoryRepository repo;
    repo.setAllDrinks(inventoryList);

    //????₯? λ¦¬μ€?Έ λ°ν
    auto resultList = repo.getList();
    EXPECT_EQ(resultList.size(), 20);

    // μ²? λ²μ§Έ ?λ£? κ²?μ¦?
    EXPECT_EQ(resultList[0].first, "μ½λΌ");
    EXPECT_EQ(resultList[0].second, 10);

    // λ§μ??λ§? ?λ£? κ²?μ¦?
    EXPECT_EQ(resultList[19].first, "μΉΈν?????");
    EXPECT_EQ(resultList[19].second, 10);

    // μ€κ° ?λ£? κ²?μ¦?
    EXPECT_EQ(resultList[9].first, "λΉν??500");
    EXPECT_EQ(resultList[9].second, 10);

}

