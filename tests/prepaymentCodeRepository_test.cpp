// #include <gtest/gtest.h>
// #include "../include/persistence/prepayCodeRepository.h"
// #include "../include/domain/prepaymentCode.h"
// #include "../include/domain/order.h"
// #include "../include/domain/drink.h"


// TEST(PrepaymentCodeRepositoryTest, MultipleSaves) {
//     PrepaymentCodeRepository repo;
//     std::vector<PrepaymentCode> codes;
    
//     // ?ฌ?ฌ ๊ฐ์ ์ฝ๋ ????ฅ
//     for (int i = 0; i < 5; i++) {
//         PrepaymentCode code;
//         codes.push_back(code);
//         repo.save(code);
//     }
    
//     // ????ฅ? ์ฝ๋ ์ค? ??๋ฅ? ? ???ฌ ๊ฒ?์ฆ?
//     EXPECT_TRUE(repo.isSameCode(codes[2].getCode()));
// }

// TEST(PrepaymentCodeRepositoryTest, InvalidCode) {
//     PrepaymentCodeRepository repo;
    
//     for (int i = 0; i < 5; i++) {
//         PrepaymentCode code;
//         repo.save(code); 
//     }

//     // ์กด์ฌ?์ง? ?? ์ฝ๋ ๊ฒ??
//     EXPECT_FALSE(repo.isSameCode("INVALID"));
//     EXPECT_FALSE(repo.isSameCode(""));
//     EXPECT_FALSE(repo.isSameCode("ABC12"));  // ????ฅ?์ง? ???? ? ?จ? ??? ์ฝ๋
// } 