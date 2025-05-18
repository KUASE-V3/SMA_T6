#include <gtest/gtest.h>
#include "../include/persistence/prepayCodeRepository.h"
#include "../include/domain/prepaymentCode.h"
#include "../include/domain/order.h"
#include "../include/domain/drink.h"


TEST(PrepaymentCodeRepositoryTest, MultipleSaves) {
    PrepaymentCodeRepository repo;
    
    // 여러 개의 코드 저장
    for (int i = 0; i < 5; i++) {
        PrepaymentCode code;
        repo.save(code);
        // 저장 즉시 존재하는지 확인
        EXPECT_TRUE(repo.isSameCode(code.getCode()));
    }
}

TEST(PrepaymentCodeRepositoryTest, InvalidCode) {
    PrepaymentCodeRepository repo;
    
    // 존재하지 않는 코드 검색
    EXPECT_FALSE(repo.isSameCode("INVALID"));
    EXPECT_FALSE(repo.isSameCode(""));
    EXPECT_FALSE(repo.isSameCode("ABC12"));  // 저장되지 않은 유효한 형식의 코드
} 