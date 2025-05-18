#include <gtest/gtest.h>
#include "../include/domain/order.h"
#include "../include/domain/drink.h"

// 기본 생성자 테스트
TEST(OrderTest, DefaultConstructor) {
    Order order;
    // 기본 생성자에서 상태가 "Pending"인지 확인
    // (getStatus 메소드가 있다면 사용)
    EXPECT_EQ(order.getPayStatus(), "Pending");
}

// 매개변수 생성자 테스트
TEST(OrderTest, ParameterizedConstructor) {
    Drink drink("콜라", 1500, "D001");
    Order order(drink, "ORDER001", "Pending");
    
    // 주문 정보가 올바르게 설정되었는지 확인
    EXPECT_EQ(order.getDrink().getName(), "콜라");
    EXPECT_EQ(order.getDrink().getPrice(), 1500);
    EXPECT_EQ(order.getDrink().getCode(), "D001");
    EXPECT_EQ(order.getOrderID(), "ORDER001");
    EXPECT_EQ(order.getPayStatus(), "Pending");
}

// attachPrePay 메소드 테스트
TEST(OrderTest, AttachPrePay) {
    Drink drink("사이다", 1500, "D002");
    std::string code = "PRE001";
    
    Order order = Order().attachPrePay(drink, code);
    
    // attachPrePay로 생성된 주문이 올바른지 확인
    EXPECT_EQ(order.getDrink().getName(), "사이다");
    EXPECT_EQ(order.getDrink().getPrice(), 1500);
    EXPECT_EQ(order.getDrink().getCode(), "D002");
    EXPECT_EQ(order.getOrderID(), code);
    EXPECT_EQ(order.getPayStatus(), "Pending");
}

// setStatus 메소드 테스트
TEST(OrderTest, SetStatus) {
    Order order;
    
    // 유효한 상태로 변경
    order.setStatus("Approved");
    EXPECT_EQ(order.getPayStatus(), "Approved");
    
    order.setStatus("Declined");
    EXPECT_EQ(order.getPayStatus(), "Declined");
    
    // 잘못된 상태는 무시되어야 함
    order.setStatus("Invalid");
    EXPECT_EQ(order.getPayStatus(), "Declined");  // 이전 상태 유지
}
