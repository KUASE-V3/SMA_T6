#include <gtest/gtest.h>
#include "../include/domain/prepaymentCode.h"
#include "../include/domain/order.h"
#include "../include/domain/drink.h"

// 생성자 테스트
TEST(PrepaymentCodeTest, Constructor) {
    PrepaymentCode code;
    // 생성자에서 code가 5자리이고 상태가 "Unused"인지 확인
    EXPECT_EQ(code.getCode().length(), 5);
    EXPECT_TRUE(PrepaymentCode::isUsable(code.getCode()));
}

// rand() 메소드 테스트
TEST(PrepaymentCodeTest, Rand) {
    PrepaymentCode code1;
    PrepaymentCode code2;
    // 두 개의 다른 코드가 생성되는지 확인
    EXPECT_NE(code1.getCode(), code2.getCode());
    
    // 생성된 코드가 올바른 형식인지 확인
    std::string code = code1.getCode();
    EXPECT_EQ(code.length(), 5);
    for (char c : code) {
        EXPECT_TRUE(std::isalnum(c));
        if (std::isalpha(c)) {
            EXPECT_TRUE(std::isupper(c));
        }
    }
}

// isUsable() 메소드 테스트
TEST(PrepaymentCodeTest, IsUsable) {
    // 올바른 형식의 코드
    std::string validCode = "ABC12";
    EXPECT_TRUE(PrepaymentCode::isUsable(validCode));

    // 잘못된 길이
    std::string wrongLength = "ABC1";
    EXPECT_FALSE(PrepaymentCode::isUsable(wrongLength));

    // 소문자 포함
    std::string lowerCase = "abc12";
    EXPECT_FALSE(PrepaymentCode::isUsable(lowerCase));

    // 특수문자 포함
    std::string specialChar = "ABC!2";
    EXPECT_FALSE(PrepaymentCode::isUsable(specialChar));
}

// hold() 메소드 테스트
TEST(PrepaymentCodeTest, Hold) {
    PrepaymentCode code;
    Drink drink("콜라", 1500, "D001");
    Order order(drink, "ORDER001", "Pending");
    
    PrepaymentCode heldCode = code.hold(order);
    // hold 메소드가 현재 객체를 반환하는지 확인
    EXPECT_EQ(heldCode.getCode(), code.getCode());
}

// setStatus() 메소드 테스트
TEST(PrepaymentCodeTest, SetStatus) {
    PrepaymentCode code;
    std::string usedStatus = "Used";
    
    // 유효한 상태 변경
    code.setStatus(usedStatus);
    // 상태가 "Used"로 변경되었는지 확인
    EXPECT_EQ(usedStatus, code.getStatus());
    
    // (getStatus 메소드가 있다면 사용)
}

// getCode() 메소드 테스트
TEST(PrepaymentCodeTest, GetCode) {
    PrepaymentCode code;
    std::string generatedCode = code.getCode();
    
    // getCode가 올바른 형식의 코드를 반환하는지 확인
    EXPECT_EQ(generatedCode.length(), 5);
    EXPECT_TRUE(PrepaymentCode::isUsable(generatedCode));
}
