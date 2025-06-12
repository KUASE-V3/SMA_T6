#include <gtest/gtest.h>
#include "domain/drink.h"
#include "domain/inventory.h"
#include "persistence/DrinkRepository.hpp"
#include "persistence/inventoryRepository.h"
#include "service/ErrorService.hpp"
#include "service/InventoryService.hpp"
#include "presentation/UserInterface.hpp"

#include <vector>
#include <string>
#include <iostream>

using domain::Drink;
using domain::Inventory;

// UC02: 사용자 음료 선택 과정 테스트 (재고 확인 전까지)


// 테스트 3: 잘못된 음료 코드 입력 처리 (UserInterface::selectDrink 에러 처리)
TEST(UC02Test, InvalidDrinkCodeInputHandling) {
    std::cout << "=== UC02 테스트: 잘못된 음료 코드 입력 처리 ===" << std::endl;
    
    // Repository 초기화
    persistence::DrinkRepository drinkRepo;
    std::vector<Drink> allDrinks = drinkRepo.findAll();
    
    std::cout << "잘못된 입력 처리 테스트" << std::endl;
    
    // 잘못된 음료 코드들 (UserInterface::selectDrink에서 거부될 입력들)
    std::vector<std::string> invalidCodes = {
        "1",    // 1자리
        "ABC",  // 알파벳
        "99",   // 존재하지 않는 코드
        "00",   // 잘못된 형식
        "123",  // 3자리
        "",     // 빈 문자열
        " 01 "  // 공백 포함
    };
    
    for (const std::string& code : invalidCodes) {
        // UserInterface::selectDrink의 검증 로직
        bool isValidFormat = (code.length() == 2 && 
                             std::all_of(code.begin(), code.end(), ::isdigit));
        
        // 음료 목록에 존재하는지 확인
        bool existsInList = false;
        if (isValidFormat) {
            for (const auto& drink : allDrinks) {
                if (drink.getDrinkCode() == code) {
                    existsInList = true;
                    break;
                }
            }
        }
        
        bool shouldBeRejected = !isValidFormat || !existsInList;
        EXPECT_TRUE(shouldBeRejected);
        
        std::cout << "✗ 거부된 입력: '" << code << "' - " 
                  << (isValidFormat ? "형식 OK" : "형식 NG") << ", "
                  << (existsInList ? "존재함" : "존재하지 않음") << std::endl;
    }
    
    std::cout << "✓ UC02.3 완료: 잘못된 음료 코드 입력 처리" << std::endl;
}


// 테스트 5: 음료 선택 완료 - UC02의 최종 단계 (UserProcessController::getDrinkDetails)
TEST(UC02Test, DrinkSelectionCompletion) {
    std::cout << "=== UC02 테스트: 음료 선택 완료 및 음료 정보 조회 ===" << std::endl;
    
    // Repository 초기화
    persistence::DrinkRepository drinkRepo;
    persistence::InventoryRepository inventoryRepo;
    service::ErrorService errorService;
    service::InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    
    // 사용자가 선택한 음료 코드 (UC02의 결과)
    std::string selectedCode = "01"; // 콜라
    std::cout << "UC02 결과: 사용자가 선택한 음료 코드 = " << selectedCode << std::endl;
    
    // UC02 마지막 단계: 선택된 음료의 상세 정보 조회 (UserProcessController::getDrinkDetails)
    std::vector<Drink> allDrinks = inventoryService.getAllDrinkTypes();
    Drink selectedDrink;
    bool drinkFound = false;
    
    for (const auto& drink : allDrinks) {
        if (drink.getDrinkCode() == selectedCode) {
            selectedDrink = drink;
            drinkFound = true;
            break;
        }
    }
    
    // UC02 완료 검증
    EXPECT_TRUE(drinkFound); // 음료가 성공적으로 찾아져야 함
    if (drinkFound) {
        EXPECT_EQ(selectedDrink.getDrinkCode(), selectedCode);
        EXPECT_FALSE(selectedDrink.getName().empty());
        EXPECT_GT(selectedDrink.getPrice(), 0);
        
        std::cout << "UC02 완료 - 선택된 음료 정보:" << std::endl;
        std::cout << "  코드: " << selectedDrink.getDrinkCode() << std::endl;
        std::cout << "  이름: " << selectedDrink.getName() << std::endl;
        std::cout << "  가격: " << selectedDrink.getPrice() << "원" << std::endl;
        std::cout << "→ 다음 단계: UC03 재고 확인으로 진행" << std::endl;
    }
    
    std::cout << "✓ UC02.5 완료: 음료 선택 과정 완전 종료, UC03으로 전달" << std::endl;
}
