// #include <gtest/gtest.h>
// #include "../include/domain/prepaymentCode.h"
// #include "../include/domain/order.h"
// #include "../include/domain/drink.h"

// using namespace domain;


// // ?��?��?�� ?��?��?��
// TEST(PrepaymentCodeTest, Constructor) {
//     PrepaymentCode code;
//     // ?��?��?��?��?�� code�? 5?��리이�? ?��?���? "Unused"?���? ?��?��
//     EXPECT_EQ(code.getCode().length(), 5);
//     EXPECT_TRUE(PrepaymentCode::isUsable(code.getCode()));
// }

// // rand() 메소?�� ?��?��?��
// TEST(PrepaymentCodeTest, Rand) {
//     PrepaymentCode code1;
//     PrepaymentCode code2;
//     // ?�� 개의 ?���? 코드�? ?��?��?��?���? ?��?��
//     EXPECT_NE(code1.getCode(), code2.getCode());
    
//     // ?��?��?�� 코드�? ?��바른 ?��?��?���? ?��?��
//     std::string code = code1.getCode();
//     EXPECT_EQ(code.length(), 5);
//     for (char c : code) {
//         EXPECT_TRUE(std::isalnum(c));
//         if (std::isalpha(c)) {
//             EXPECT_TRUE(std::isupper(c));
//         }
//     }
// }

// // isUsable() 메소?�� ?��?��?��
// TEST(PrepaymentCodeTest, IsUsable) {
//     // ?��바른 ?��?��?�� 코드
//     std::string validCode = "ABC12";
//     EXPECT_TRUE(PrepaymentCode::isUsable(validCode));

//     // ?��못된 길이
//     std::string wrongLength = "ABC1";
//     EXPECT_FALSE(PrepaymentCode::isUsable(wrongLength));

//     // ?��문자 ?��?��
//     std::string lowerCase = "abc12";
//     EXPECT_FALSE(PrepaymentCode::isUsable(lowerCase));

//     // ?��?��문자 ?��?��
//     std::string specialChar = "ABC!2";
//     EXPECT_FALSE(PrepaymentCode::isUsable(specialChar));
// }

// // hold() 메소?�� ?��?��?��
// TEST(PrepaymentCodeTest, Hold) {
//     PrepaymentCode code;
//     Drink drink("콜라", 1500, "D001");
//     Order order(drink, "ORDER001", "Pending");
    
//     PrepaymentCode heldCode = code.hold(order);
//     // hold 메소?���? ?��?�� 객체�? 반환?��?���? ?��?��
//     EXPECT_EQ(heldCode.getCode(), code.getCode());
// }

// // setStatus() 메소?�� ?��?��?��
// TEST(PrepaymentCodeTest, SetStatus) {
//     PrepaymentCode code;
//     std::string usedStatus = "Used";
    
//     // ?��?��?�� ?��?�� �?�?
//     code.setStatus(usedStatus);
//     // ?��?���? "Used"�? �?경되?��?���? ?��?��
//     EXPECT_EQ(usedStatus, code.getStatus());
    
//     // (getStatus 메소?���? ?��?���? ?��?��)
// }

// // getCode() 메소?�� ?��?��?��
// TEST(PrepaymentCodeTest, GetCode) {
//     PrepaymentCode code;
//     std::string generatedCode = code.getCode();
    
//     // getCode�? ?��바른 ?��?��?�� 코드�? 반환?��?���? ?��?��
//     EXPECT_EQ(generatedCode.length(), 5);
//     EXPECT_TRUE(PrepaymentCode::isUsable(generatedCode));
// }
