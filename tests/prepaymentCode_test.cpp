// #include <gtest/gtest.h>
// #include "../include/domain/prepaymentCode.h"
// #include "../include/domain/order.h"
// #include "../include/domain/drink.h"

// using namespace domain;


// // ??±? ??€?Έ
// TEST(PrepaymentCodeTest, Constructor) {
//     PrepaymentCode code;
//     // ??±??? codeκ°? 5?λ¦¬μ΄κ³? ??κ°? "Unused"?Έμ§? ??Έ
//     EXPECT_EQ(code.getCode().length(), 5);
//     EXPECT_TRUE(PrepaymentCode::isUsable(code.getCode()));
// }

// // rand() λ©μ? ??€?Έ
// TEST(PrepaymentCodeTest, Rand) {
//     PrepaymentCode code1;
//     PrepaymentCode code2;
//     // ? κ°μ ?€λ₯? μ½λκ°? ??±??μ§? ??Έ
//     EXPECT_NE(code1.getCode(), code2.getCode());
    
//     // ??±? μ½λκ°? ?¬λ°λ₯Έ ???Έμ§? ??Έ
//     std::string code = code1.getCode();
//     EXPECT_EQ(code.length(), 5);
//     for (char c : code) {
//         EXPECT_TRUE(std::isalnum(c));
//         if (std::isalpha(c)) {
//             EXPECT_TRUE(std::isupper(c));
//         }
//     }
// }

// // isUsable() λ©μ? ??€?Έ
// TEST(PrepaymentCodeTest, IsUsable) {
//     // ?¬λ°λ₯Έ ??? μ½λ
//     std::string validCode = "ABC12";
//     EXPECT_TRUE(PrepaymentCode::isUsable(validCode));

//     // ?λͺ»λ κΈΈμ΄
//     std::string wrongLength = "ABC1";
//     EXPECT_FALSE(PrepaymentCode::isUsable(wrongLength));

//     // ?λ¬Έμ ?¬?¨
//     std::string lowerCase = "abc12";
//     EXPECT_FALSE(PrepaymentCode::isUsable(lowerCase));

//     // ?Ή?λ¬Έμ ?¬?¨
//     std::string specialChar = "ABC!2";
//     EXPECT_FALSE(PrepaymentCode::isUsable(specialChar));
// }

// // hold() λ©μ? ??€?Έ
// TEST(PrepaymentCodeTest, Hold) {
//     PrepaymentCode code;
//     Drink drink("μ½λΌ", 1500, "D001");
//     Order order(drink, "ORDER001", "Pending");
    
//     PrepaymentCode heldCode = code.hold(order);
//     // hold λ©μ?κ°? ??¬ κ°μ²΄λ₯? λ°ν??μ§? ??Έ
//     EXPECT_EQ(heldCode.getCode(), code.getCode());
// }

// // setStatus() λ©μ? ??€?Έ
// TEST(PrepaymentCodeTest, SetStatus) {
//     PrepaymentCode code;
//     std::string usedStatus = "Used";
    
//     // ? ?¨? ?? λ³?κ²?
//     code.setStatus(usedStatus);
//     // ??κ°? "Used"λ‘? λ³?κ²½λ??μ§? ??Έ
//     EXPECT_EQ(usedStatus, code.getStatus());
    
//     // (getStatus λ©μ?κ°? ??€λ©? ?¬?©)
// }

// // getCode() λ©μ? ??€?Έ
// TEST(PrepaymentCodeTest, GetCode) {
//     PrepaymentCode code;
//     std::string generatedCode = code.getCode();
    
//     // getCodeκ°? ?¬λ°λ₯Έ ??? μ½λλ₯? λ°ν??μ§? ??Έ
//     EXPECT_EQ(generatedCode.length(), 5);
//     EXPECT_TRUE(PrepaymentCode::isUsable(generatedCode));
// }
