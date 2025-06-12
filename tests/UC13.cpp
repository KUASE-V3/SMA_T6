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

// UC13: 인증코드 입력 검증 테스트

// 테스트 1: 인증코드 입력 검증 - 잘못된 경우들
TEST(UC13Test, AuthCodeInputValidation_InvalidCases) {
    persistence::PrepayCodeRepository prepayRepo;
    persistence::OrderRepository orderRepo;
    service::ErrorService errorService;
    service::PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    
    // 1. 잘못된 형식 - 길이가 짧음 (4자리)
    std::string shortCode = "AB12";
    EXPECT_FALSE(prepaymentService.isValidAuthCodeFormat(shortCode));
    
    // 2. 잘못된 형식 - 길이가 김 (6자리)
    std::string longCode = "AB1234";
    EXPECT_FALSE(prepaymentService.isValidAuthCodeFormat(longCode));
    
    // 3. 잘못된 형식 - 특수문자 포함
    std::string specialCharCode = "AB1@#";
    EXPECT_FALSE(prepaymentService.isValidAuthCodeFormat(specialCharCode));
    
    // 4. 잘못된 형식 - 공백 포함
    std::string spaceCode = "AB 12";
    EXPECT_FALSE(prepaymentService.isValidAuthCodeFormat(spaceCode));
    
    // 5. 잘못된 형식 - 빈 문자열
    std::string emptyCode = "";
    EXPECT_FALSE(prepaymentService.isValidAuthCodeFormat(emptyCode));
    
    // 6. 형식은 맞지만 존재하지 않는 인증코드
    std::string nonExistentCode = "XYZ99";
    EXPECT_TRUE(prepaymentService.isValidAuthCodeFormat(nonExistentCode)); // 형식은 올바름
    
    PrePaymentCode result = prepaymentService.getPrepaymentDetailsIfActive(nonExistentCode);
    EXPECT_TRUE(result.getCode().empty()); // 존재하지 않으므로 빈 객체 반환
}

// 테스트 2: 인증코드 입력 검증 - 올바른 경우
TEST(UC13Test, AuthCodeInputValidation_ValidCase) {
    persistence::PrepayCodeRepository prepayRepo;
    persistence::OrderRepository orderRepo;
    service::ErrorService errorService;
    service::PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    
    // 1. 유효한 인증코드 생성 및 등록
    auto order = std::make_shared<Order>("T1", "01", 1, "", "APPROVED");
    std::string validAuthCode = prepaymentService.generateAuthCodeString();
    order->setCertCode(validAuthCode);
    prepaymentService.registerPrepayment(validAuthCode, order);
    
    // 2. 형식 검증 - 통과해야 함
    EXPECT_TRUE(prepaymentService.isValidAuthCodeFormat(validAuthCode));
    EXPECT_EQ(validAuthCode.length(), 5);
    
    // 3. 생성된 코드가 알파벳과 숫자로만 구성되어 있는지 확인
    for (char c : validAuthCode) {
        EXPECT_TRUE(std::isalnum(c)) << "Invalid character in auth code: " << c;
    }
    
    // 4. 존재하고 사용 가능한 인증코드 검증 - 통과해야 함
    PrePaymentCode activePrepayCode = prepaymentService.getPrepaymentDetailsIfActive(validAuthCode);
    EXPECT_FALSE(activePrepayCode.getCode().empty());
    EXPECT_EQ(activePrepayCode.getCode(), validAuthCode);
    EXPECT_TRUE(activePrepayCode.isUsable());
    
    // 5. 연결된 주문 정보 확인
    auto linkedOrder = activePrepayCode.getHeldOrder();
    ASSERT_NE(linkedOrder, nullptr);
    EXPECT_EQ(linkedOrder->getDrinkCode(), "01");
    EXPECT_EQ(linkedOrder->getCertCode(), validAuthCode);
}

// 테스트 3: 이미 사용된 인증코드 테스트
TEST(UC13Test, AuthCodeInputValidation_UsedCode) {
    persistence::PrepayCodeRepository prepayRepo;
    persistence::OrderRepository orderRepo;
    service::ErrorService errorService;
    service::PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    
    // 1. 유효한 인증코드 생성 및 등록
    auto order = std::make_shared<Order>("T1", "01", 1, "", "APPROVED");
    std::string authCode = prepaymentService.generateAuthCodeString();
    order->setCertCode(authCode);
    prepaymentService.registerPrepayment(authCode, order);
    
    // 2. 처음에는 사용 가능해야 함
    PrePaymentCode beforeUse = prepaymentService.getPrepaymentDetailsIfActive(authCode);
    EXPECT_FALSE(beforeUse.getCode().empty());
    EXPECT_TRUE(beforeUse.isUsable());
    
    // 3. 인증코드를 사용됨으로 변경
    prepaymentService.changeAuthCodeStatusToUsed(authCode);
    
    // 4. 사용된 후에는 사용 불가능해야 함
    PrePaymentCode afterUse = prepaymentService.getPrepaymentDetailsIfActive(authCode);
    EXPECT_TRUE(afterUse.getCode().empty()); // 사용된 코드는 빈 객체 반환
}

// 테스트 4: 다양한 유효한 형식의 인증코드 테스트
TEST(UC13Test, AuthCodeFormat_VariousValidFormats) {
    persistence::PrepayCodeRepository prepayRepo;
    persistence::OrderRepository orderRepo;
    service::ErrorService errorService;
    service::PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    
    // 다양한 유효한 5자리 영숫자 조합 테스트
    std::vector<std::string> validCodes = {
        "ABC12",  // 대문자 + 숫자
        "abc12",  // 소문자 + 숫자  
        "12345",  // 숫자만
        "ABCDE",  // 대문자만
        "abcde",  // 소문자만
        "A1b2C",  // 혼합
        "9Z8y7"   // 숫자와 문자 혼합
    };
    
    for (const auto& code : validCodes) {
        EXPECT_TRUE(prepaymentService.isValidAuthCodeFormat(code)) 
            << "Code should be valid: " << code;
    }
}

// 테스트 5: 통합 시나리오 - 사용자 인증코드 입력 프로세스
TEST(UC13Test, IntegratedAuthCodeInputProcess) {
    persistence::PrepayCodeRepository prepayRepo;
    persistence::OrderRepository orderRepo;
    service::ErrorService errorService;
    service::PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    
    // 1단계: 선결제로 인증코드 발급
    auto order = std::make_shared<Order>("T2", "02", 1, "", "APPROVED");
    std::string issuedAuthCode = prepaymentService.generateAuthCodeString();
    order->setCertCode(issuedAuthCode);
    prepaymentService.registerPrepayment(issuedAuthCode, order);
    
    std::cout << "발급된 인증코드: " << issuedAuthCode << std::endl;
    
    // 2단계: 사용자가 자판기에서 잘못된 코드 입력 시도
    std::string userInput1 = "123";  // 너무 짧음
    EXPECT_FALSE(prepaymentService.isValidAuthCodeFormat(userInput1));
    std::cout << "잘못된 입력 '" << userInput1 << "': 형식 검증 실패" << std::endl;
    
    std::string userInput2 = "ABC@#";  // 특수문자 포함
    EXPECT_FALSE(prepaymentService.isValidAuthCodeFormat(userInput2));
    std::cout << "잘못된 입력 '" << userInput2 << "': 형식 검증 실패" << std::endl;
    
    // 3단계: 사용자가 올바른 인증코드 입력
    EXPECT_TRUE(prepaymentService.isValidAuthCodeFormat(issuedAuthCode));
    PrePaymentCode validResult = prepaymentService.getPrepaymentDetailsIfActive(issuedAuthCode);
    EXPECT_FALSE(validResult.getCode().empty());
    EXPECT_TRUE(validResult.isUsable());
    std::cout << "올바른 코드 '" << issuedAuthCode << "': 검증 성공, 음료 제공 가능" << std::endl;
    
    // 4단계: 음료 제공 후 코드 사용 처리
    prepaymentService.changeAuthCodeStatusToUsed(issuedAuthCode);
    PrePaymentCode usedResult = prepaymentService.getPrepaymentDetailsIfActive(issuedAuthCode);
    EXPECT_TRUE(usedResult.getCode().empty()); // 사용된 코드는 더 이상 유효하지 않음
    std::cout << "코드 '" << issuedAuthCode << "': 사용 완료, 더 이상 유효하지 않음" << std::endl;
    
    std::cout << "✓ 인증코드 입력 검증 프로세스 테스트 완료" << std::endl;
}
