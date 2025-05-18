// #include <gtest/gtest.h>
// #include "../include/domain/prepaymentCode.h"
// #include "../include/domain/order.h"
// #include "../include/domain/drink.h"

// using namespace domain;


// // ?ƒ?„±? ?…Œ?Š¤?Š¸
// TEST(PrepaymentCodeTest, Constructor) {
//     PrepaymentCode code;
//     // ?ƒ?„±??—?„œ codeê°? 5?ë¦¬ì´ê³? ?ƒ?ƒœê°? "Unused"?¸ì§? ?™•?¸
//     EXPECT_EQ(code.getCode().length(), 5);
//     EXPECT_TRUE(PrepaymentCode::isUsable(code.getCode()));
// }

// // rand() ë©”ì†Œ?“œ ?…Œ?Š¤?Š¸
// TEST(PrepaymentCodeTest, Rand) {
//     PrepaymentCode code1;
//     PrepaymentCode code2;
//     // ?‘ ê°œì˜ ?‹¤ë¥? ì½”ë“œê°? ?ƒ?„±?˜?Š”ì§? ?™•?¸
//     EXPECT_NE(code1.getCode(), code2.getCode());
    
//     // ?ƒ?„±?œ ì½”ë“œê°? ?˜¬ë°”ë¥¸ ?˜•?‹?¸ì§? ?™•?¸
//     std::string code = code1.getCode();
//     EXPECT_EQ(code.length(), 5);
//     for (char c : code) {
//         EXPECT_TRUE(std::isalnum(c));
//         if (std::isalpha(c)) {
//             EXPECT_TRUE(std::isupper(c));
//         }
//     }
// }

// // isUsable() ë©”ì†Œ?“œ ?…Œ?Š¤?Š¸
// TEST(PrepaymentCodeTest, IsUsable) {
//     // ?˜¬ë°”ë¥¸ ?˜•?‹?˜ ì½”ë“œ
//     std::string validCode = "ABC12";
//     EXPECT_TRUE(PrepaymentCode::isUsable(validCode));

//     // ?˜ëª»ëœ ê¸¸ì´
//     std::string wrongLength = "ABC1";
//     EXPECT_FALSE(PrepaymentCode::isUsable(wrongLength));

//     // ?†Œë¬¸ì ?¬?•¨
//     std::string lowerCase = "abc12";
//     EXPECT_FALSE(PrepaymentCode::isUsable(lowerCase));

//     // ?Š¹?ˆ˜ë¬¸ì ?¬?•¨
//     std::string specialChar = "ABC!2";
//     EXPECT_FALSE(PrepaymentCode::isUsable(specialChar));
// }

// // hold() ë©”ì†Œ?“œ ?…Œ?Š¤?Š¸
// TEST(PrepaymentCodeTest, Hold) {
//     PrepaymentCode code;
//     Drink drink("ì½œë¼", 1500, "D001");
//     Order order(drink, "ORDER001", "Pending");
    
//     PrepaymentCode heldCode = code.hold(order);
//     // hold ë©”ì†Œ?“œê°? ?˜„?¬ ê°ì²´ë¥? ë°˜í™˜?•˜?Š”ì§? ?™•?¸
//     EXPECT_EQ(heldCode.getCode(), code.getCode());
// }

// // setStatus() ë©”ì†Œ?“œ ?…Œ?Š¤?Š¸
// TEST(PrepaymentCodeTest, SetStatus) {
//     PrepaymentCode code;
//     std::string usedStatus = "Used";
    
//     // ?œ ?š¨?•œ ?ƒ?ƒœ ë³?ê²?
//     code.setStatus(usedStatus);
//     // ?ƒ?ƒœê°? "Used"ë¡? ë³?ê²½ë˜?—ˆ?Š”ì§? ?™•?¸
//     EXPECT_EQ(usedStatus, code.getStatus());
    
//     // (getStatus ë©”ì†Œ?“œê°? ?ˆ?‹¤ë©? ?‚¬?š©)
// }

// // getCode() ë©”ì†Œ?“œ ?…Œ?Š¤?Š¸
// TEST(PrepaymentCodeTest, GetCode) {
//     PrepaymentCode code;
//     std::string generatedCode = code.getCode();
    
//     // getCodeê°? ?˜¬ë°”ë¥¸ ?˜•?‹?˜ ì½”ë“œë¥? ë°˜í™˜?•˜?Š”ì§? ?™•?¸
//     EXPECT_EQ(generatedCode.length(), 5);
//     EXPECT_TRUE(PrepaymentCode::isUsable(generatedCode));
// }
