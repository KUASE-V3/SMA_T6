// #include <gtest/gtest.h>
// #include "../include/persistence/prepayCodeRepository.h"
// #include "../include/domain/prepaymentCode.h"
// #include "../include/domain/order.h"
// #include "../include/domain/drink.h"


// TEST(PrepaymentCodeRepositoryTest, MultipleSaves) {
//     PrepaymentCodeRepository repo;
//     std::vector<PrepaymentCode> codes;
    
//     // ?��?�� 개의 코드 ????��
//     for (int i = 0; i < 5; i++) {
//         PrepaymentCode code;
//         codes.push_back(code);
//         repo.save(code);
//     }
    
//     // ????��?�� 코드 �? ?��?���? ?��?��?��?�� �?�?
//     EXPECT_TRUE(repo.isSameCode(codes[2].getCode()));
// }

// TEST(PrepaymentCodeRepositoryTest, InvalidCode) {
//     PrepaymentCodeRepository repo;
    
//     for (int i = 0; i < 5; i++) {
//         PrepaymentCode code;
//         repo.save(code); 
//     }

//     // 존재?���? ?��?�� 코드 �??��
//     EXPECT_FALSE(repo.isSameCode("INVALID"));
//     EXPECT_FALSE(repo.isSameCode(""));
//     EXPECT_FALSE(repo.isSameCode("ABC12"));  // ????��?���? ?��??? ?��?��?�� ?��?��?�� 코드
// } 