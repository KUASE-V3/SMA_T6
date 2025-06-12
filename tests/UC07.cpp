#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <memory>

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

// Presentation
#include "presentation/UserInterface.hpp"

using namespace domain;
using namespace service;
using namespace persistence;
using namespace presentation;





// 테스트 3: 음료 배출 UI 메시지 테스트
TEST(UC07Test, DrinkDispensingUIMessages) {
    std::cout << "=== UC07 테스트: 음료 배출 UI 메시지 ===" << std::endl;
    
    UserInterface ui;
    std::string testDrinkName = "콜라";
    
    // UC7.1: 배출 시작 메시지
    std::cout << "배출 시작 메시지 테스트..." << std::endl;
    EXPECT_NO_THROW({
        ui.displayDispensingDrink(testDrinkName);
    });
    
    // UC7.3: 배출 완료 메시지  
    std::cout << "배출 완료 메시지 테스트..." << std::endl;
    EXPECT_NO_THROW({
        ui.displayDrinkDispensed(testDrinkName);
    });
    
    std::cout << "✓ UC07.3 완료: 음료 배출 UI 메시지 테스트" << std::endl;
}

// 테스트 4: 재고 차감 로직 상세 테스트
TEST(UC07Test, StockReductionLogic) {
    std::cout << "=== UC07 테스트: 재고 차감 로직 상세 ===" << std::endl;
    
    // Repository 및 Service 초기화
    InventoryRepository inventoryRepo;
    DrinkRepository drinkRepo;
    ErrorService errorService;
    InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    
    // 초기 재고 설정
    inventoryRepo.addOrUpdateStock(Inventory("01", 3)); // 콜라 3개
    
    std::cout << "초기 재고: 3개" << std::endl;
    auto initialStock = inventoryService.checkDrinkAvailabilityAndPrice("01");
    EXPECT_EQ(initialStock.currentStock, 3);
    EXPECT_TRUE(initialStock.isAvailable);
    
    // UC7.2: 일반 구매 재고 차감 (OrderService::processOrderApproval에서 수행)
    inventoryService.decreaseStock("01");
    
    auto afterFirstDecrease = inventoryService.checkDrinkAvailabilityAndPrice("01");
    std::cout << "1차 차감 후 재고: " << afterFirstDecrease.currentStock << "개" << std::endl;
    EXPECT_EQ(afterFirstDecrease.currentStock, 2);
    EXPECT_TRUE(afterFirstDecrease.isAvailable);
    
    // 추가 차감
    inventoryService.decreaseStock("01");
    inventoryService.decreaseStock("01");
    
    auto afterAllDecrease = inventoryService.checkDrinkAvailabilityAndPrice("01");
    std::cout << "전체 차감 후 재고: " << afterAllDecrease.currentStock << "개" << std::endl;
    EXPECT_EQ(afterAllDecrease.currentStock, 0);
    EXPECT_FALSE(afterAllDecrease.isAvailable);
    
    std::cout << "✓ UC07.4 완료: 재고 차감 로직 검증" << std::endl;
}