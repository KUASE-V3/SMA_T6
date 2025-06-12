#include <gtest/gtest.h>
#include "domain/drink.h"
#include "domain/inventory.h"
#include "persistence/DrinkRepository.hpp"
#include "persistence/inventoryRepository.h"
#include "service/ErrorService.hpp"
#include "service/InventoryService.hpp"

#include <vector>
#include <string>
#include <iostream>

using domain::Drink;
using domain::Inventory;

// UC03: 현재 자판기 재고 확인 과정 테스트 (UC02에서 음료 선택 완료 후)

// 테스트 1: UC02 완료 후 재고 있는 음료의 재고 확인
TEST(UC03Test, CheckStockForAvailableDrink) {
    std::cout << "=== UC03 테스트: 재고 있는 음료 재고 확인 ===" << std::endl;
    
    // Repository 및 Service 초기화
    persistence::InventoryRepository inventoryRepo;
    persistence::DrinkRepository drinkRepo;
    service::ErrorService errorService;
    service::InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    
    // T1 자판기 재고 설정 (main.cpp와 동일)
    inventoryRepo.addOrUpdateStock(Inventory("01", 5));  // 콜라 5개
    inventoryRepo.addOrUpdateStock(Inventory("02", 0));  // 사이다 0개
    inventoryRepo.addOrUpdateStock(Inventory("03", 10)); // 녹차 10개
    inventoryRepo.addOrUpdateStock(Inventory("04", 2));  // 홍차 2개
    
    std::cout << "T1 자판기 재고: 콜라(5), 사이다(0), 녹차(10), 홍차(2)" << std::endl;
    
    // UC02 완료: 사용자가 콜라(01)를 선택 완료
    std::string selectedDrinkCode = "01";
    std::cout << "UC02 완료 → UC03 시작: 선택된 음료 = " << selectedDrinkCode << " (콜라)" << std::endl;
    
    // UC03: 현재 자판기 재고 확인 (UserProcessController::state_awaitingDrinkSelection과 동일)
    auto availability = inventoryService.checkDrinkAvailabilityAndPrice(selectedDrinkCode);
    
    // 재고 확인 결과 검증
    EXPECT_TRUE(availability.isAvailable);    // 구매 가능해야 함
    EXPECT_EQ(availability.currentStock, 5);  // 재고 5개
    EXPECT_GT(availability.price, 0);         // 가격 정보 있어야 함
    
    std::cout << "재고 확인 결과:" << std::endl;
    std::cout << "  구매 가능: " << (availability.isAvailable ? "YES" : "NO") << std::endl;
    std::cout << "  현재 재고: " << availability.currentStock << "개" << std::endl;
    std::cout << "  가격: " << availability.price << "원" << std::endl;
    
}




