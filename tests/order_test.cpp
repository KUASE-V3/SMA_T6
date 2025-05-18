// #include <gtest/gtest.h>
// #include "../include/domain/order.h"
// #include "../include/domain/drink.h"

// // ê¸°ë³¸ ?ƒ?„±? ?…Œ?Š¤?Š¸
// TEST(OrderTest, DefaultConstructor) {
//     Order order;
//     // ê¸°ë³¸ ?ƒ?„±??—?„œ ?ƒ?ƒœê°? "Pending"?¸ì§? ?™•?¸
//     // (getStatus ë©”ì†Œ?“œê°? ?ˆ?‹¤ë©? ?‚¬?š©)
//     EXPECT_EQ(order.getPayStatus(), "Pending");
// }

// // ë§¤ê°œë³??ˆ˜ ?ƒ?„±? ?…Œ?Š¤?Š¸
// TEST(OrderTest, ParameterizedConstructor) {
//     Drink drink("ì½œë¼", 1500, "D001");
//     Order order(drink, "ORDER001", "Pending");
    
//     // ì£¼ë¬¸ ? •ë³´ê?? ?˜¬ë°”ë¥´ê²? ?„¤? •?˜?—ˆ?Š”ì§? ?™•?¸
//     EXPECT_EQ(order.getDrink().getName(), "ì½œë¼");
//     EXPECT_EQ(order.getDrink().getPrice(), 1500);
//     EXPECT_EQ(order.getDrink().getCode(), "D001");
//     EXPECT_EQ(order.getOrderID(), "ORDER001");
//     EXPECT_EQ(order.getPayStatus(), "Pending");
// }

// // attachPrePay ë©”ì†Œ?“œ ?…Œ?Š¤?Š¸
// TEST(OrderTest, AttachPrePay) {
//     Drink drink("?‚¬?´?‹¤", 1500, "D002");
//     std::string code = "PRE001";
    
//     Order order = Order().attachPrePay(drink, code);
    
//     // attachPrePayë¡? ?ƒ?„±?œ ì£¼ë¬¸?´ ?˜¬ë°”ë¥¸ì§? ?™•?¸
//     EXPECT_EQ(order.getDrink().getName(), "?‚¬?´?‹¤");
//     EXPECT_EQ(order.getDrink().getPrice(), 1500);
//     EXPECT_EQ(order.getDrink().getCode(), "D002");
//     EXPECT_EQ(order.getOrderID(), code);
//     EXPECT_EQ(order.getPayStatus(), "Pending");
// }

// // setStatus ë©”ì†Œ?“œ ?…Œ?Š¤?Š¸
// TEST(OrderTest, SetStatus) {
//     Order order;
    
//     // ?œ ?š¨?•œ ?ƒ?ƒœë¡? ë³?ê²?
//     order.setStatus("Approved");
//     EXPECT_EQ(order.getPayStatus(), "Approved");
    
//     order.setStatus("Declined");
//     EXPECT_EQ(order.getPayStatus(), "Declined");
    
//     // ?˜ëª»ëœ ?ƒ?ƒœ?Š” ë¬´ì‹œ?˜?–´?•¼ ?•¨
//     order.setStatus("Invalid");
//     EXPECT_EQ(order.getPayStatus(), "Declined");  // ?´? „ ?ƒ?ƒœ ?œ ì§?
// }
