#include <gtest/gtest.h>
#include "domain/order.h"
#include "domain/prepaymentCode.h"

#include "persistence/prepayCodeRepository.h"
#include "persistence/OrderRepository.hpp"

#include "service/ErrorService.hpp"
#include "service/PrepaymentService.hpp"

#include <memory>
#include <string>

using domain::Order;
using domain::PrePaymentCode;

// UC14: 인증코드 검증 및 처리 테스트 (3가지 핵심 시나리오)

// 테스트 1: 형식 검증은 통과하되 존재하지 않는 인증코드 검증 테스트
TEST(UC14Test, ValidFormatButNonExistentAuthCode) {
    persistence::PrepayCodeRepository prepayRepo;
    persistence::OrderRepository orderRepo;
    service::ErrorService errorService;
    service::PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    
    // 형식은 올바르지만 시스템에 등록되지 않은 인증코드
    std::string nonExistentCode = "ABCD1"; // 5자리 알파벳+숫자 형식
    
    // 1. 형식 검증은 통과해야 함
    EXPECT_TRUE(prepaymentService.isValidAuthCodeFormat(nonExistentCode));
    std::cout << "형식 검증 통과: " << nonExistentCode << std::endl;
    
    // 2. 존재하지 않는 코드이므로 빈 객체 반환되어야 함
    PrePaymentCode result = prepaymentService.getPrepaymentDetailsIfActive(nonExistentCode);
    EXPECT_TRUE(result.getCode().empty());
    
    std::cout << "존재하지 않는 코드 '" << nonExistentCode << "': 검증 실패, 코드를 찾을 수 없음" << std::endl;
    std::cout << "✓ 테스트 1 완료: 형식은 유효하나 존재하지 않는 코드 처리" << std::endl;
}

// 테스트 2: 이미 사용된 인증코드를 또 사용했을 경우 테스트
TEST(UC14Test, AlreadyUsedAuthCodeVerification) {
    persistence::PrepayCodeRepository prepayRepo;
    persistence::OrderRepository orderRepo;
    service::ErrorService errorService;
    service::PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    
    // 1. 유효한 인증코드 생성 및 등록
    auto order = std::make_shared<Order>("T1", "01", 1, "", "APPROVED");
    std::string authCode = prepaymentService.generateAuthCodeString();
    order->setCertCode(authCode);
    prepaymentService.registerPrepayment(authCode, order);
    
    std::cout << "인증코드 생성 및 등록: " << authCode << std::endl;
    
    // 2. 첫 번째 사용 - 정상 처리
    PrePaymentCode firstUse = prepaymentService.getPrepaymentDetailsIfActive(authCode);
    EXPECT_FALSE(firstUse.getCode().empty());
    EXPECT_TRUE(firstUse.isUsable());
    
    // 음료 제공 후 코드를 사용됨으로 처리
    prepaymentService.changeAuthCodeStatusToUsed(authCode);
    std::cout << "첫 번째 사용 완료, 코드 상태를 'used'로 변경" << std::endl;
    
    // 3. 두 번째 사용 시도 - 실패해야 함
    PrePaymentCode secondUse = prepaymentService.getPrepaymentDetailsIfActive(authCode);
    EXPECT_TRUE(secondUse.getCode().empty()); // 이미 사용된 코드는 빈 객체 반환
    
    std::cout << "두 번째 사용 시도: 코드 '" << authCode << "'는 이미 사용되어 무효함" << std::endl;
    std::cout << "✓ 테스트 2 완료: 이미 사용된 코드 재사용 방지" << std::endl;
}

// 테스트 3: 알맞게 사용했을 경우 인증코드 상태가 used로 바뀌었는지 확인
TEST(UC14Test, ProperUsageAndStatusChangeToUsed) {
    persistence::PrepayCodeRepository prepayRepo;
    persistence::OrderRepository orderRepo;
    service::ErrorService errorService;
    service::PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    
    // 1. 유효한 인증코드 생성 및 등록
    auto order = std::make_shared<Order>("T2", "02", 1, "", "APPROVED");
    std::string authCode = prepaymentService.generateAuthCodeString();
    order->setCertCode(authCode);
    prepaymentService.registerPrepayment(authCode, order);
    
    std::cout << "인증코드 생성: " << authCode << " (음료코드: " << order->getDrinkCode() << ")" << std::endl;
    
    // 2. 사용 전 상태 확인 - 활성 상태여야 함
    PrePaymentCode beforeUse = prepaymentService.getPrepaymentDetailsIfActive(authCode);
    EXPECT_FALSE(beforeUse.getCode().empty());
    EXPECT_TRUE(beforeUse.isUsable());
    
    auto linkedOrder = beforeUse.getHeldOrder();
    ASSERT_NE(linkedOrder, nullptr);
    EXPECT_EQ(linkedOrder->getDrinkCode(), "02");
    
    std::cout << "사용 전 상태: 코드 활성화됨, 주문 정보 조회 가능" << std::endl;
    
    // 3. 정상적인 음료 제공 과정 시뮬레이션
    std::cout << "음료 제공 중..." << std::endl;
    
    // 4. 음료 제공 완료 후 인증코드를 사용됨 상태로 변경
    prepaymentService.changeAuthCodeStatusToUsed(authCode);
    std::cout << "음료 제공 완료, 인증코드 상태를 'used'로 변경" << std::endl;
    
    // 5. 사용 후 상태 확인 - Repository에서 직접 상태를 확인
    PrePaymentCode afterUse = prepayRepo.findByCode(authCode);
    EXPECT_FALSE(afterUse.getCode().empty()); // 코드는 여전히 존재함
    EXPECT_EQ(afterUse.getStatus(), domain::CodeStatus::USED); // 상태가 USED로 변경되었는지 확인
    EXPECT_FALSE(afterUse.isUsable()); // 더 이상 사용 불가능해야 함
    
    std::cout << "사용 후 상태: 코드 상태가 'USED'로 정확히 변경됨" << std::endl;
    std::cout << "✓ 테스트 3 완료: 정상 사용 후 상태가 USED로 변경되었음을 직접 확인" << std::endl;
}
