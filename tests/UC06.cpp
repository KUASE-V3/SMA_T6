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


// 테스트 1: 결제 거절 처리
TEST(UC06Test, PaymentDeclinationProcessing) {
    std::cout << "=== UC06 테스트: 결제 거절 처리 ===" << std::endl;
    
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
    
    // 주문 생성
    Drink selectedDrink = drinkRepo.findByDrinkCode("01");
    Order testOrder = orderService.createOrder("T1", selectedDrink);
    
    std::cout << "주문 생성: " << selectedDrink.getName() << " (상태: " << testOrder.getPayStatus() << ")" << std::endl;
    EXPECT_EQ(testOrder.getPayStatus(), "PENDING");
    
    // 결제 전 재고 확인
    auto beforeStock = inventoryService.checkDrinkAvailabilityAndPrice("01");
    std::cout << "결제 전 재고: " << beforeStock.currentStock << "개" << std::endl;
    
    // UC6: 결제 거절 처리
    orderService.processOrderDeclination(testOrder);
    
    // 결과 검증
    EXPECT_EQ(testOrder.getPayStatus(), "DECLINED");
    
    // 재고 변화 없음 확인
    auto afterStock = inventoryService.checkDrinkAvailabilityAndPrice("01");
    std::cout << "결제 후 재고: " << afterStock.currentStock << "개" << std::endl;
    EXPECT_EQ(afterStock.currentStock, beforeStock.currentStock); // 재고 변화 없어야 함
    
    std::cout << "✓ UC06.1 완료: 결제 거절 처리" << std::endl;
}

// 테스트 2: 결제 거절 후 UI 메시지 표시
TEST(UC06Test, PaymentDeclinationUIDisplay) {
    std::cout << "=== UC06 테스트: 결제 거절 UI 표시 ===" << std::endl;
    
    presentation::UserInterface ui;
    
    // UC6.3: 결제 실패 메시지 표시
    std::cout << "결제 실패 메시지 표시 테스트..." << std::endl;
    
    EXPECT_NO_THROW({
        ui.displayPaymentResult(false, "결제에 실패했습니다.");
    });
    
    std::cout << "✓ UC06.2 완료: 결제 거절 UI 메시지 표시" << std::endl;
}