#include <gtest/gtest.h>
#include "../include/persistence/prepayCodeRepository.h"
#include "../include/domain/prepaymentCode.h"
#include "../include/domain/order.h"
#include "../include/domain/drink.h"


TEST(PrepaymentCodeRepositoryTest, MultipleSaves) {
    PrepaymentCodeRepository repo;
    std::vector<PrepaymentCode> codes;
    
    // 여러 개의 코드 저장
    for (int i = 0; i < 5; i++) {
        PrepaymentCode code;
        codes.push_back(code);
        repo.save(code);
    }
    
    // 저장된 코드 중 하나를 선택하여 검증
    EXPECT_TRUE(repo.isSameCode(codes[2].getCode()));
}

TEST(PrepaymentCodeRepositoryTest, InvalidCode) {
    PrepaymentCodeRepository repo;
    
    for (int i = 0; i < 5; i++) {
        PrepaymentCode code;
        repo.save(code); 
    }

    // 존재하지 않는 코드 검색
    EXPECT_FALSE(repo.isSameCode("INVALID"));
    EXPECT_FALSE(repo.isSameCode(""));
    EXPECT_FALSE(repo.isSameCode("ABC12"));  // 저장되지 않은 유효한 형식의 코드
} 