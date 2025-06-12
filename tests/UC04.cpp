#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <thread>

// Domain
#include "domain/drink.h"
#include "domain/order.h"
#include "domain/inventory.h"

// Persistence
#include "persistence/DrinkRepository.hpp"
#include "persistence/inventoryRepository.h"
#include "persistence/OrderRepository.hpp"
#include "persistence/prepayCodeRepository.h"

// Service
#include "service/ErrorService.hpp"
#include "service/InventoryService.hpp"
#include "service/OrderService.hpp"
#include "service/PrepaymentService.hpp"

// Network
#include "network/PaymentCallbackReceiver.hpp"

// Presentation
#include "presentation/UserInterface.hpp"

using namespace domain;
using namespace service;
using namespace persistence;
using namespace network;
using namespace presentation;

TEST(UC04Test, PaymentSimulationBasicOperation) {
    std::cout << "=== UC04 테스트: 결제 시뮬레이션 기본 동작 ===" << std::endl;
    
    PaymentCallbackReceiver paymentSim;
    bool paymentResult = false;
    bool callbackCalled = false;
    
    std::cout << "결제 시뮬레이션 시작 (1초 지연)..." << std::endl;
    auto startTime = std::chrono::steady_clock::now();
    
    // UC4.3: 결제 시뮬레이션 실행
    paymentSim.simulatePrepayment([&](bool success) {
        paymentResult = success;
        callbackCalled = true;
        std::cout << "결제 시뮬레이션 완료: " << (success ? "성공" : "실패") << std::endl;
    }, 1); // 1초 지연
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // 검증
    EXPECT_TRUE(callbackCalled);
    EXPECT_GE(duration.count(), 1000); // 최소 1초 지연 확인
    EXPECT_LT(duration.count(), 1200); // 1.2초 이내 완료 확인
    
    std::cout << "지연 시간: " << duration.count() << "ms" << std::endl;
    std::cout << "✓ UC04 완료: 결제 시뮬레이션 기본 동작 확인" << std::endl;
}


// 테스트 3: UserInterface 결제 확인 프롬프트 메소드 테스트
TEST(UC04Test, PaymentConfirmationPrompt) {
    std::cout << "=== UC04 테스트: 결제 확인 프롬프트 ===" << std::endl;
    
    presentation::UserInterface ui;
    
    // UC4.1: 결제 프롬프트 표시 (실제 사용자 입력 없이 메소드 호출만 테스트)
    std::cout << "결제 프롬프트 표시 테스트..." << std::endl;
    
    // 이 메소드들은 사용자 입력을 받으므로 실제 호출은 하지 않고, 
    // 메소드 존재 여부와 기본 파라미터 처리 확인
    int testPrice = 1500;
    
    // displayPaymentPrompt는 void 반환이므로 예외 없이 실행되는지만 확인
    EXPECT_NO_THROW({
        ui.displayPaymentPrompt(testPrice);
    });
    
    // displayPaymentProcessing도 마찬가지
    EXPECT_NO_THROW({
        ui.displayPaymentProcessing();
    });
    
    std::cout << "✓ UC04 완료: 결제 프롬프트 메소드 확인" << std::endl;
}