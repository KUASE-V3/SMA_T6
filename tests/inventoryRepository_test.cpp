#include <gtest/gtest.h>
#include "../include/persistence/inventoryRepository.h"
#include "../include/domain/drink.h"
#include "../include/domain/inventory.h"


using domain::Drink;
using domain::inventory;
using persistence::inventoryRepository;

TEST(InventoryRepositoryTest, SetAndGetAllDrinks) {
    std::vector<Drink> testDrinks = {
        Drink("ì½œë¼", 1500, "D001"),
        Drink("?‚¬?´?‹¤", 1500, "D002"),
        Drink("?™˜??? ?˜¤? Œì§?", 1600, "D003"),
        Drink("?™˜??? ?¬?„", 1600, "D004"),
        Drink("ëª¬ìŠ¤?„°", 2500, "D005"),
        Drink("? ˆ?“œë¶?", 2700, "D006"),
        Drink("ê²Œí† ? ˆ?´", 1800, "D007"),
        Drink("?ŒŒ?›Œ?—?´?“œ", 1800, "D008"),
        Drink("?´?”„ë¡?", 1700, "D009"),
        Drink("ë¹„í??500", 1000, "D010"),
        Drink("ë³´ì„±?…¹ì°?", 1500, "D011"),
        Drink("?˜¥?ˆ˜?ˆ˜?ˆ˜?—¼ì°?", 1500, "D012"),
        Drink("ì½”ì½”?Œœ", 1600, "D013"),
        Drink("?Š¸ë¡œí”¼ì¹´ë‚˜", 1600, "D014"),
        Drink("?•«?‹?Š¤", 2000, "D015"),
        Drink("?•„ë©”ë¦¬ì¹´ë…¸ ìº”ì»¤?”¼", 1200, "D016"),
        Drink("ì¡°ì???•„ ?˜¤ë¦¬ì???„", 1300, "D017"),
        Drink("TOP ?Š¤?œ„?Š¸ ?•„ë©”ë¦¬ì¹´ë…¸", 1400, "D018"),
        Drink("ë°??‚¤?Š¤", 1500, "D019"),
        Drink("ì¹¸í?????", 1600, "D020")
    };

    //?¸ë²¤í† ë¦? ë¦¬ìŠ¤?Š¸ ?ƒ?„±
    std::vector<inventory> inventoryList;
    const int INITIAL_QUANTITY = 10;  // ì´ˆê¸° ?¬ê³? ?ˆ˜?Ÿ‰

    for (const auto& drink : testDrinks) {
        inventoryList.push_back(inventory(drink, INITIAL_QUANTITY));
    }

    EXPECT_EQ(inventoryList.size(), 20);

    EXPECT_EQ(inventoryList[0].getDrink().getName(), "ì½œë¼");
    EXPECT_EQ(inventoryList[0].getDrink().getPrice(), 1500);
    EXPECT_EQ(inventoryList[0].getDrink().getCode(), "D001");
    EXPECT_EQ(inventoryList[0].getQty(), 10);

    

    //? ˆ?¼ì§??† ë¦? ê°ì²´ ?ƒ?„± ?›„ ? •?–ˆ?˜ ?¸ë²¤í† ë¦?(?Œë£? + ?ˆ˜?Ÿ‰) ?‚´?š© ????¥
    inventoryRepository repo;
    repo.setAllDrinks(inventoryList);

    //????¥?œ ë¦¬ìŠ¤?Š¸ ë°˜í™˜
    auto resultList = repo.getList();
    EXPECT_EQ(resultList.size(), 20);

    // ì²? ë²ˆì§¸ ?Œë£? ê²?ì¦?
    EXPECT_EQ(resultList[0].first, "ì½œë¼");
    EXPECT_EQ(resultList[0].second, 10);

    // ë§ˆì??ë§? ?Œë£? ê²?ì¦?
    EXPECT_EQ(resultList[19].first, "ì¹¸í?????");
    EXPECT_EQ(resultList[19].second, 10);

    // ì¤‘ê°„ ?Œë£? ê²?ì¦?
    EXPECT_EQ(resultList[9].first, "ë¹„í??500");
    EXPECT_EQ(resultList[9].second, 10);

}

