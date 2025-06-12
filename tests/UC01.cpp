#include <gtest/gtest.h>
#include "service/InventoryService.hpp"
#include "persistence/inventoryRepository.h"
#include "persistence/DrinkRepository.hpp"
#include "presentation/UserInterface.hpp"
#include "service/ErrorService.hpp"
#include <vector>
#include <algorithm>
#include <iostream>

using namespace service;
using namespace persistence;
using namespace domain;

// 테스트 1: 음료 목록 표시 기능 테스트
TEST(UC02Test, DisplayDrinkMenu) {
    std::cout << "=== UC02 테스트: 음료 목록 표시 기능 ===" << std::endl;
    
    // Repository 및 Service 초기화
    persistence::DrinkRepository drinkRepo;
    persistence::InventoryRepository inventoryRepo;
    service::ErrorService errorService;
    service::InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    presentation::UserInterface ui;
    
    // 전체 음료 목록 가져오기 (main.cpp의 displayMainMenu와 동일)
    std::vector<Drink> allDrinks = inventoryService.getAllDrinkTypes();
    
    std::cout << "전체 음료 수: " << allDrinks.size() << "종" << std::endl;
    
    // 음료 목록이 올바르게 로드되었는지 확인
    EXPECT_GT(allDrinks.size(), 0); // 최소 1개 이상의 음료
    
    // 주요 음료들이 포함되어 있는지 확인
    bool hasBasicDrinks = false;
    for (const auto& drink : allDrinks) {
        if (drink.getDrinkCode() == "01" || drink.getDrinkCode() == "02" || drink.getDrinkCode() == "03") {
            hasBasicDrinks = true;
            EXPECT_FALSE(drink.getName().empty());
            EXPECT_GT(drink.getPrice(), 0);
            std::cout << "음료: " << drink.getDrinkCode() << " - " << drink.getName() 
                      << " (" << drink.getPrice() << "원)" << std::endl;
        }
    }
    EXPECT_TRUE(hasBasicDrinks);
    
    // UI 표시 기능 테스트 (실제 출력은 하지 않음)
    std::vector<Inventory> emptyInventory; // 빈 재고 정보
    ui.displayDrinkList(allDrinks, emptyInventory);
    
    std::cout << "✓ 테스트 1 완료: 음료 목록 표시 기능 정상" << std::endl;
}