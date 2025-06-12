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

// --- UC05 테스트들 ---

// 테스트 1: 일반 구매 결제 승인 처리
TEST(UC05Test, RegularPurchaseApprovalProcessing) {
    std::cout << "=== UC05 테스트: 일반 구매 결제 승인 처리 ===" << std::endl;
    
    // Repository 및 Service 초기화
    InventoryRepository inventoryRepo;
    DrinkRepository drinkRepo;
    OrderRepository orderRepo;
    PrepayCodeRepository prepayRepo;
    ErrorService errorService;
    
    InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    OrderService orderService(orderRepo, drinkRepo, inventoryService, prepaymentService, errorService);
    
    // 초기 재고 설정
    inventoryRepo.addOrUpdateStock(Inventory("01", 5)); // 콜라 5개
    
    // 주문 생성 (UC2, UC3 완료 후 상태)
    Drink selectedDrink = drinkRepo.findByDrinkCode("01");
    Order testOrder = orderService.createOrder("T1", selectedDrink);
    
    std::cout << "주문 생성: " << selectedDrink.getName() << " (상태: " << testOrder.getPayStatus() << ")" << std::endl;
    EXPECT_EQ(testOrder.getPayStatus(), "PENDING");
    
    // 결제 전 재고 확인
    auto beforeStock = inventoryService.checkDrinkAvailabilityAndPrice("01");
    std::cout << "결제 전 재고: " << beforeStock.currentStock << "개" << std::endl;
    
    // UC5: 결제 승인 처리 (일반 구매)
    orderService.processOrderApproval(testOrder, false); // isPrepayment = false
    
    // 결과 검증
    EXPECT_EQ(testOrder.getPayStatus(), "APPROVED");
    
    // 재고 차감 확인
    auto afterStock = inventoryService.checkDrinkAvailabilityAndPrice("01");
    std::cout << "결제 후 재고: " << afterStock.currentStock << "개" << std::endl;
    EXPECT_EQ(afterStock.currentStock, beforeStock.currentStock - 1);
    
    std::cout << "✓ UC05.1 완료: 일반 구매 결제 승인 처리" << std::endl;
}

// 테스트 2: 선결제 결제 승인 처리 (인증코드 생성)
TEST(UC05Test, PrepaymentApprovalProcessing) {
    std::cout << "=== UC05 테스트: 선결제 결제 승인 처리 ===" << std::endl;
    
    // Repository 및 Service 초기화
    InventoryRepository inventoryRepo;
    DrinkRepository drinkRepo;
    OrderRepository orderRepo;
    PrepayCodeRepository prepayRepo;
    ErrorService errorService;
    
    InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    OrderService orderService(orderRepo, drinkRepo, inventoryService, prepaymentService, errorService);
    
    // 주문 생성 (선결제용)
    Drink selectedDrink = drinkRepo.findByDrinkCode("02");
    Order testOrder = orderService.createOrder("T1", selectedDrink);
    
    std::cout << "선결제 주문 생성: " << selectedDrink.getName() << " (상태: " << testOrder.getPayStatus() << ")" << std::endl;
    EXPECT_EQ(testOrder.getPayStatus(), "PENDING");
    EXPECT_TRUE(testOrder.getCertCode().empty()); // 초기에는 인증코드 없음
    
    // UC5: 선결제 승인 처리
    orderService.processOrderApproval(testOrder, true); // isPrepayment = true
    
    // 결과 검증
    EXPECT_EQ(testOrder.getPayStatus(), "APPROVED");
    EXPECT_FALSE(testOrder.getCertCode().empty()); // 인증코드 생성됨
    EXPECT_EQ(testOrder.getCertCode().length(), 5); // 5자리 인증코드
    
    std::cout << "생성된 인증코드: " << testOrder.getCertCode() << std::endl;
    
    // PrepaymentService에 등록 확인
    PrePaymentCode savedPrepayCode = prepayRepo.findByCode(testOrder.getCertCode());
    EXPECT_EQ(savedPrepayCode.getCode(), testOrder.getCertCode());
    EXPECT_TRUE(savedPrepayCode.isUsable());
    
    // 연결된 주문 정보 확인
    auto savedOrder = savedPrepayCode.getHeldOrder();
    ASSERT_NE(savedOrder, nullptr);
    EXPECT_EQ(savedOrder->getDrinkCode(), selectedDrink.getDrinkCode());
    EXPECT_EQ(savedOrder->getCertCode(), testOrder.getCertCode());
    
    std::cout << "✓ UC05.2 완료: 선결제 승인 처리 및 인증코드 생성" << std::endl;
}

// 테스트 3: 결제 승인 후 UI 메시지 표시
TEST(UC05Test, PaymentApprovalUIDisplay) {
    std::cout << "=== UC05 테스트: 결제 승인 UI 표시 ===" << std::endl;
    
    presentation::UserInterface ui;
    
    // UC5.3: 결제 성공 메시지 표시
    std::cout << "결제 성공 메시지 표시 테스트..." << std::endl;
    
    EXPECT_NO_THROW({
        ui.displayPaymentResult(true, "결제가 성공적으로 완료되었습니다!");
    });
    
    std::cout << "✓ UC05.3 완료: 결제 승인 UI 메시지 표시" << std::endl;
}