// #include <gtest/gtest.h>
// #include "../include/domain/order.h"
// #include "../include/domain/drink.h"

// // 기본 ?��?��?�� ?��?��?��
// TEST(OrderTest, DefaultConstructor) {
//     Order order;
//     // 기본 ?��?��?��?��?�� ?��?���? "Pending"?���? ?��?��
//     // (getStatus 메소?���? ?��?���? ?��?��)
//     EXPECT_EQ(order.getPayStatus(), "Pending");
// }

// // 매개�??�� ?��?��?�� ?��?��?��
// TEST(OrderTest, ParameterizedConstructor) {
//     Drink drink("콜라", 1500, "D001");
//     Order order(drink, "ORDER001", "Pending");
    
//     // 주문 ?��보�?? ?��바르�? ?��?��?��?��?���? ?��?��
//     EXPECT_EQ(order.getDrink().getName(), "콜라");
//     EXPECT_EQ(order.getDrink().getPrice(), 1500);
//     EXPECT_EQ(order.getDrink().getCode(), "D001");
//     EXPECT_EQ(order.getOrderID(), "ORDER001");
//     EXPECT_EQ(order.getPayStatus(), "Pending");
// }

// // attachPrePay 메소?�� ?��?��?��
// TEST(OrderTest, AttachPrePay) {
//     Drink drink("?��?��?��", 1500, "D002");
//     std::string code = "PRE001";
    
//     Order order = Order().attachPrePay(drink, code);
    
//     // attachPrePay�? ?��?��?�� 주문?�� ?��바른�? ?��?��
//     EXPECT_EQ(order.getDrink().getName(), "?��?��?��");
//     EXPECT_EQ(order.getDrink().getPrice(), 1500);
//     EXPECT_EQ(order.getDrink().getCode(), "D002");
//     EXPECT_EQ(order.getOrderID(), code);
//     EXPECT_EQ(order.getPayStatus(), "Pending");
// }

// // setStatus 메소?�� ?��?��?��
// TEST(OrderTest, SetStatus) {
//     Order order;
    
//     // ?��?��?�� ?��?���? �?�?
//     order.setStatus("Approved");
//     EXPECT_EQ(order.getPayStatus(), "Approved");
    
//     order.setStatus("Declined");
//     EXPECT_EQ(order.getPayStatus(), "Declined");
    
//     // ?��못된 ?��?��?�� 무시?��?��?�� ?��
//     order.setStatus("Invalid");
//     EXPECT_EQ(order.getPayStatus(), "Declined");  // ?��?�� ?��?�� ?���?
// }
