#include <gtest/gtest.h>
#include "../include/domain/drink.h"
#include "../include/domain/inventory.h"
#include "../include/domain/order.h"
#include "../include/domain/vendingMachine.h"
#include "../include/domain/prepaymentCode.h"

#include "../include/persistence/DrinkRepository.hpp"
#include "../include/persistence/inventoryRepository.h"
#include "../include/persistence/OrderRepository.hpp"
#include "../include/persistence/OvmAddressRepository.hpp"
#include "../include/persistence/prepayCodeRepository.h"

#include "../include/service/ErrorService.hpp"
#include "../include/service/InventoryService.hpp"
#include "../include/service/DistanceService.hpp"
#include "../include/service/PrepaymentService.hpp"
#include "../include/service/OrderService.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cassert>

using domain::VendingMachine;
using domain::Drink;
using domain::Inventory;
using domain::Order;
using domain::PrePaymentCode;
using service::DistanceService;

// 테스트용 자판기 정보 (T1, T2, T3만 사용)
const std::vector<VendingMachine> TEST_VENDING_MACHINES = {
    VendingMachine("T1", 10, 10, "12345"),
    VendingMachine("T2", 20, 20, "12346"),
    VendingMachine("T3", 10, 30, "12347")
};



// 거리 계산 함수
double calculateDistance(int x1, int y1, int x2, int y2) {
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}

// 가장 가까운 자판기 찾기 함수
VendingMachine findNearestVendingMachine(int userX, int userY, const std::vector<VendingMachine>& machines) {
    if (machines.empty()) {
        return VendingMachine(); // 빈 객체 반환
    }
    
    auto nearest = machines[0];
    double minDistance = calculateDistance(userX, userY, nearest.getLocation().first, nearest.getLocation().second);
    
    for (const auto& machine : machines) {
        double distance = calculateDistance(userX, userY, machine.getLocation().first, machine.getLocation().second);
        if (distance < minDistance) {
            minDistance = distance;
            nearest = machine;
        }
    }
    
    return nearest;
}

// 테스트 1: 인증코드 생성 테스트
TEST(UC12Test, AuthCodeGeneration) {
    persistence::PrepayCodeRepository prepayRepo;
    persistence::OrderRepository orderRepo;
    service::ErrorService errorService;
    service::PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    
    // 인증코드 생성
    std::string authCode = prepaymentService.generateAuthCodeString();
    
    // 인증코드가 5자리 영숫자인지 확인
    EXPECT_EQ(authCode.length(), 5);
    EXPECT_TRUE(prepaymentService.isValidAuthCodeFormat(authCode));
    
    // 인증코드가 빈 문자열이 아닌지 확인
    EXPECT_FALSE(authCode.empty());
}

// 테스트 2a: 가까운 자판기 찾기 테스트 - 사용자 위치 (20,20)
TEST(UC12Test, FindNearestVendingMachine_Location20_20) {
    // 사용자 위치 설정 (20, 20)
    int userX = 20, userY = 20;
    
    // 가장 가까운 자판기 찾기
    VendingMachine nearest = findNearestVendingMachine(userX, userY, TEST_VENDING_MACHINES);
    
    // 가장 가까운 자판기가 찾아졌는지 확인
    EXPECT_FALSE(nearest.getId().empty());
    
    // 거리 계산
    double distance = calculateDistance(userX, userY, nearest.getLocation().first, nearest.getLocation().second);
    
    // T2(20,20)와 동일한 위치이므로 거리는 0
    EXPECT_EQ(distance, 0.0);
    EXPECT_EQ(nearest.getId(), "T2");
}

// 테스트 2b: 가까운 자판기 찾기 테스트 - 사용자 위치 (30,30)
TEST(UC12Test, FindNearestVendingMachine_Location30_30) {
    // 사용자 위치 설정 (30, 30)
    int userX = 30, userY = 30;
    
    // 가장 가까운 자판기 찾기
    VendingMachine nearest = findNearestVendingMachine(userX, userY, TEST_VENDING_MACHINES);
    
    // 가장 가까운 자판기가 찾아졌는지 확인
    EXPECT_FALSE(nearest.getId().empty());
    
    // 거리 계산
    double distance = calculateDistance(userX, userY, nearest.getLocation().first, nearest.getLocation().second);
    
    // T2(20,20)가 가장 가까움 (거리 약 14.14)
    EXPECT_NEAR(distance, 14.14, 0.1);
    EXPECT_EQ(nearest.getId(), "T2");
}

// 테스트 3: 자판기 재고 확인 테스트
TEST(UC12Test, VendingMachineInventoryCheck) {
    persistence::DrinkRepository drinkRepo;
    persistence::InventoryRepository inventoryRepo;
    service::ErrorService errorService;
    service::InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    
    // T1 자판기 재고 설정 (메인 함수와 동일)
    inventoryRepo.addOrUpdateStock(Inventory("01", 5));  // 콜라
    inventoryRepo.addOrUpdateStock(Inventory("02", 0));  // 사이다 (재고 없음)
    inventoryRepo.addOrUpdateStock(Inventory("03", 10)); // 녹차
    
    // 콜라 재고 확인
    auto colaInventory = inventoryRepo.getInventoryByDrinkCode("01");
    EXPECT_EQ(colaInventory.getDrinkCode(), "01");
    EXPECT_EQ(colaInventory.getQty(), 5);
    
    // 사이다 재고 확인 (재고 없음)
    auto ciderInventory = inventoryRepo.getInventoryByDrinkCode("02");
    EXPECT_EQ(ciderInventory.getDrinkCode(), "02");
    EXPECT_EQ(ciderInventory.getQty(), 0);
    
    // 음료 가용성 확인
    auto colaAvailability = inventoryService.checkDrinkAvailabilityAndPrice("01");
    EXPECT_TRUE(colaAvailability.isAvailable);
    EXPECT_EQ(colaAvailability.currentStock, 5);
    EXPECT_EQ(colaAvailability.price, 1000);
    
    auto ciderAvailability = inventoryService.checkDrinkAvailabilityAndPrice("02");
    EXPECT_FALSE(ciderAvailability.isAvailable);
    EXPECT_EQ(ciderAvailability.currentStock, 0);
}

// 테스트 4: 선결제 등록 테스트
TEST(UC12Test, PrepaymentRegistration) {
    persistence::PrepayCodeRepository prepayRepo;
    persistence::OrderRepository orderRepo;
    service::ErrorService errorService;
    service::PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    
    // 주문 생성
    auto order = std::make_shared<Order>("T1", "01", 1, "", "APPROVED");
    
    // 인증코드 생성
    std::string authCode = prepaymentService.generateAuthCodeString();
    order->setCertCode(authCode);
    
    // 선결제 등록
    prepaymentService.registerPrepayment(authCode, order);
    
    // 등록된 선결제 정보 확인
    PrePaymentCode savedPrepayCode = prepayRepo.findByCode(authCode);
    EXPECT_EQ(savedPrepayCode.getCode(), authCode);
    EXPECT_TRUE(savedPrepayCode.isUsable());
    
    // 연결된 주문 정보 확인
    auto savedOrder = savedPrepayCode.getHeldOrder();
    ASSERT_NE(savedOrder, nullptr);
    EXPECT_EQ(savedOrder->getDrinkCode(), "01");
    EXPECT_EQ(savedOrder->getCertCode(), authCode);
}

// 테스트 5: 통합 시나리오 테스트 (인증코드 발급 → 가까운 자판기 안내)
TEST(UC12Test, IntegratedScenario) {
    // 1단계: 서비스 설정
    persistence::PrepayCodeRepository prepayRepo;
    persistence::OrderRepository orderRepo;
    persistence::DrinkRepository drinkRepo;
    persistence::InventoryRepository inventoryRepo;
    service::ErrorService errorService;
    service::PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    service::InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    
    // 2단계: 사용자가 콜라 선택 및 주문 생성
    std::string selectedDrinkCode = "01";
    auto order = std::make_shared<Order>("T1", selectedDrinkCode, 1, "", "APPROVED");
    
    // 3단계: 인증코드 발급 및 선결제 등록
    std::string authCode = prepaymentService.generateAuthCodeString();
    order->setCertCode(authCode);
    prepaymentService.registerPrepayment(authCode, order);
    
    // 4단계: 사용자 위치에서 가까운 자판기 찾기
    int userX = 20, userY = 20;  // T2와 동일한 위치
    VendingMachine nearestVm = findNearestVendingMachine(userX, userY, TEST_VENDING_MACHINES);
    
    // 5단계: 결과 검증
    EXPECT_FALSE(authCode.empty());
    EXPECT_EQ(authCode.length(), 5);
    EXPECT_FALSE(nearestVm.getId().empty());
    
    // 선결제 정보가 올바르게 저장되었는지 확인
    PrePaymentCode savedPrepayCode = prepayRepo.findByCode(authCode);
    EXPECT_EQ(savedPrepayCode.getCode(), authCode);
    EXPECT_TRUE(savedPrepayCode.isUsable());
    
    // 가까운 자판기의 위치 정보 확인
    auto location = nearestVm.getLocation();
    EXPECT_GT(location.first, 0);
    EXPECT_GT(location.second, 0);
    
    // 거리 확인 (T2와 동일한 위치이므로 거리 0)
    double distanceToNearest = calculateDistance(userX, userY, location.first, location.second);
    EXPECT_EQ(distanceToNearest, 0.0);
    EXPECT_EQ(nearestVm.getId(), "T2");
    
    std::cout << "✓ 인증코드 " << authCode << " 발급 완료" << std::endl;
    std::cout << "✓ 가장 가까운 자판기: " << nearestVm.getId() 
              << " (위치: " << location.first << ", " << location.second << ")" << std::endl;
}


