// #include <gtest/gtest.h>
// #include "../include/persistence/prepayCodeRepository.h"
// #include "../include/domain/prepaymentCode.h"
// #include "../include/domain/order.h"
// #include "../include/domain/drink.h"


// TEST(PrepaymentCodeRepositoryTest, MultipleSaves) {
//     PrepaymentCodeRepository repo;
//     std::vector<PrepaymentCode> codes;
    
//     // ?—¬?Ÿ¬ ê°œì˜ ì½”ë“œ ????¥
//     for (int i = 0; i < 5; i++) {
//         PrepaymentCode code;
//         codes.push_back(code);
//         repo.save(code);
//     }
    
//     // ????¥?œ ì½”ë“œ ì¤? ?•˜?‚˜ë¥? ?„ ?ƒ?•˜?—¬ ê²?ì¦?
//     EXPECT_TRUE(repo.isSameCode(codes[2].getCode()));
// }

// TEST(PrepaymentCodeRepositoryTest, InvalidCode) {
//     PrepaymentCodeRepository repo;
    
//     for (int i = 0; i < 5; i++) {
//         PrepaymentCode code;
//         repo.save(code); 
//     }

//     // ì¡´ì¬?•˜ì§? ?•Š?Š” ì½”ë“œ ê²??ƒ‰
//     EXPECT_FALSE(repo.isSameCode("INVALID"));
//     EXPECT_FALSE(repo.isSameCode(""));
//     EXPECT_FALSE(repo.isSameCode("ABC12"));  // ????¥?˜ì§? ?•Š??? ?œ ?š¨?•œ ?˜•?‹?˜ ì½”ë“œ
// } 