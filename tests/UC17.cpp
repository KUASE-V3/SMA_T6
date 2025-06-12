#include <gtest/gtest.h>
#include "domain/inventory.h"
#include "domain/drink.h"

#include "persistence/inventoryRepository.h"
#include "persistence/DrinkRepository.hpp"

#include "service/ErrorService.hpp"
#include "service/InventoryService.hpp"
#include "network/message.hpp"

#include <memory>
#include <string>
#include <vector>

using domain::Inventory;
using domain::Drink;

// UC17: 다른 자판기로부터 재고 조회 요청 수신 및 응답 처리 테스트

// 테스트 1: 재고 조회 요청 수신 및 재고 정보 확인 테스트
TEST(UC17Test, ReceiveStockInquiryAndCheckInventory) {
    std::cout << "=== UC17 테스트: 재고 조회 요청 수신 및 재고 확인 ===" << std::endl;
    
    // Repository 및 Service 초기화
    persistence::InventoryRepository inventoryRepo;
    persistence::DrinkRepository drinkRepo;
    service::ErrorService errorService;
    service::InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    
    // T2 자판기 재고 설정 (main.cpp 참조)
    inventoryRepo.addOrUpdateStock(Inventory("01", 0));  // 콜라 재고 없음
    inventoryRepo.addOrUpdateStock(Inventory("02", 12)); // 사이다 12개
    inventoryRepo.addOrUpdateStock(Inventory("08", 8));  // 캔커피 8개
    inventoryRepo.addOrUpdateStock(Inventory("09", 15)); // 물 15개
    
    std::cout << "T2 자판기 재고: 콜라(0), 사이다(12), 캔커피(8), 물(15)" << std::endl;
    
    // T1에서 보낸 재고 조회 요청 메시지 시뮬레이션
    network::Message stockRequest;
    stockRequest.msg_type = network::Message::Type::REQ_STOCK;
    stockRequest.src_id = "T1";
    stockRequest.dst_id = "T2";
    stockRequest.msg_content["item_code"] = "02"; // 사이다 조회
    stockRequest.msg_content["item_num"] = "1";
    
    std::cout << "수신된 재고 조회 요청: " << stockRequest.src_id 
              << " -> " << stockRequest.dst_id 
              << ", 음료코드: " << stockRequest.msg_content.at("item_code") << std::endl;
    
    // 요청 메시지 검증
    EXPECT_EQ(stockRequest.msg_type, network::Message::Type::REQ_STOCK);
    EXPECT_EQ(stockRequest.src_id, "T1");
    EXPECT_EQ(stockRequest.dst_id, "T2");
    EXPECT_EQ(stockRequest.msg_content.at("item_code"), "02");
    
    // 재고 확인 처리
    std::string requestedDrinkCode = stockRequest.msg_content.at("item_code");
    auto availability = inventoryService.checkDrinkAvailabilityAndPrice(requestedDrinkCode);
    
    EXPECT_EQ(requestedDrinkCode, "02");
    EXPECT_TRUE(availability.isAvailable);
    EXPECT_EQ(availability.currentStock, 12);
    
    std::cout << "재고 확인 결과: 사이다 " << availability.currentStock << "개 보유" << std::endl;
    std::cout << "✓ 테스트 1 완료: 재고 조회 요청 수신 및 재고 확인 성공" << std::endl;
}

// 테스트 2: 재고 조회 응답 메시지 생성 테스트
TEST(UC17Test, GenerateStockInquiryResponse) {
    std::cout << "=== UC17 테스트: 재고 조회 응답 메시지 생성 ===" << std::endl;
    
    // Repository 및 Service 초기화
    persistence::InventoryRepository inventoryRepo;
    persistence::DrinkRepository drinkRepo;
    service::ErrorService errorService;
    service::InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    
    // T3 자판기 재고 설정
    inventoryRepo.addOrUpdateStock(Inventory("01", 3));  // 콜라 3개
    inventoryRepo.addOrUpdateStock(Inventory("13", 5));  // 기타 음료들
    inventoryRepo.addOrUpdateStock(Inventory("14", 5));
    
    std::cout << "T3 자판기 재고: 콜라(3), 기타 음료들" << std::endl;
    
    // T1으로부터 콜라 재고 조회 요청 처리
    std::string requestingVmId = "T1";
    std::string requestedDrinkCode = "01";
    
    // 재고 확인
    auto availability = inventoryService.checkDrinkAvailabilityAndPrice(requestedDrinkCode);
    
    // 응답 메시지 생성
    network::Message stockResponse;
    stockResponse.msg_type = network::Message::Type::RESP_STOCK;
    stockResponse.src_id = "T3";
    stockResponse.dst_id = requestingVmId;
    stockResponse.msg_content["item_code"] = requestedDrinkCode;
    stockResponse.msg_content["item_num"] = std::to_string(availability.currentStock);
    stockResponse.msg_content["coor_x"] = "10";  // T3 좌표 (10, 30)
    stockResponse.msg_content["coor_y"] = "30";
    
    // 응답 메시지 검증
    EXPECT_EQ(stockResponse.msg_type, network::Message::Type::RESP_STOCK);
    EXPECT_EQ(stockResponse.src_id, "T3");
    EXPECT_EQ(stockResponse.dst_id, "T1");
    EXPECT_EQ(stockResponse.msg_content.at("item_code"), "01");
    EXPECT_EQ(std::stoi(stockResponse.msg_content.at("item_num")), 3);
    EXPECT_EQ(stockResponse.msg_content.at("coor_x"), "10");
    EXPECT_EQ(stockResponse.msg_content.at("coor_y"), "30");
    
    std::cout << "생성된 응답 메시지: " << stockResponse.src_id 
              << " -> " << stockResponse.dst_id 
              << ", 콜라 재고: " << stockResponse.msg_content.at("item_num") << "개" << std::endl;
    std::cout << "좌표 정보: (" << stockResponse.msg_content.at("coor_x") 
              << ", " << stockResponse.msg_content.at("coor_y") << ")" << std::endl;
    std::cout << "✓ 테스트 2 완료: RESP_STOCK 응답 메시지 생성 성공" << std::endl;
}

// 테스트 3: 재고 없는 음료에 대한 응답 처리 테스트
TEST(UC17Test, HandleOutOfStockInquiry) {
    std::cout << "=== UC17 테스트: 재고 없는 음료 조회 요청 처리 ===" << std::endl;
    
    // Repository 및 Service 초기화
    persistence::InventoryRepository inventoryRepo;
    persistence::DrinkRepository drinkRepo;
    service::ErrorService errorService;
    service::InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    
    // T1 자판기 재고 설정 (main.cpp 참조: T1은 사이다 재고 없음)
    inventoryRepo.addOrUpdateStock(Inventory("01", 5));  // 콜라 5개
    inventoryRepo.addOrUpdateStock(Inventory("02", 0));  // 사이다 0개 (재고 없음)
    inventoryRepo.addOrUpdateStock(Inventory("03", 10)); // 녹차 10개
    
    std::cout << "T1 자판기 재고: 콜라(5), 사이다(0), 녹차(10)" << std::endl;
    
    // T5에서 사이다 재고 조회 요청
    network::Message stockRequest;
    stockRequest.msg_type = network::Message::Type::REQ_STOCK;
    stockRequest.src_id = "T5";
    stockRequest.dst_id = "T1";
    stockRequest.msg_content["item_code"] = "02"; // 사이다 (재고 없음)
    stockRequest.msg_content["item_num"] = "1";
    
    std::cout << "재고 조회 요청: T5 -> T1, 사이다 재고 문의" << std::endl;
    
    // 재고 확인
    std::string requestedDrinkCode = stockRequest.msg_content.at("item_code");
    auto availability = inventoryService.checkDrinkAvailabilityAndPrice(requestedDrinkCode);
    
    EXPECT_FALSE(availability.isAvailable);
    EXPECT_EQ(availability.currentStock, 0);
    
    // 재고 없음 응답 메시지 생성
    network::Message stockResponse;
    stockResponse.msg_type = network::Message::Type::RESP_STOCK;
    stockResponse.src_id = "T1";
    stockResponse.dst_id = "T5";
    stockResponse.msg_content["item_code"] = requestedDrinkCode;
    stockResponse.msg_content["item_num"] = "0"; // 재고 없음
    stockResponse.msg_content["coor_x"] = "10";  // T1 좌표 (10, 10)
    stockResponse.msg_content["coor_y"] = "10";
    
    // 응답 검증
    EXPECT_EQ(std::stoi(stockResponse.msg_content.at("item_num")), 0);
    EXPECT_EQ(stockResponse.msg_content.at("item_code"), "02");
    
    std::cout << "응답 메시지: 사이다 재고 " << stockResponse.msg_content.at("item_num") << "개 (재고 없음)" << std::endl;
    std::cout << "✓ 테스트 3 완료: 재고 없는 음료에 대한 응답 처리" << std::endl;
}

// 테스트 4: 여러 음료에 대한 연속 재고 조회 처리 테스트
TEST(UC17Test, HandleMultipleStockInquiries) {
    std::cout << "=== UC17 테스트: 여러 음료 재고 조회 연속 처리 ===" << std::endl;
    
    // Repository 및 Service 초기화
    persistence::InventoryRepository inventoryRepo;
    persistence::DrinkRepository drinkRepo;
    service::ErrorService errorService;
    service::InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    
    // T2 자판기 재고 설정
    inventoryRepo.addOrUpdateStock(Inventory("01", 0));  // 콜라 재고 없음
    inventoryRepo.addOrUpdateStock(Inventory("02", 12)); // 사이다 12개
    inventoryRepo.addOrUpdateStock(Inventory("08", 8));  // 캔커피 8개
    inventoryRepo.addOrUpdateStock(Inventory("09", 15)); // 물 15개
    inventoryRepo.addOrUpdateStock(Inventory("10", 3));  // 에너지드링크 3개
    
    std::cout << "T2 자판기 재고: 콜라(0), 사이다(12), 캔커피(8), 물(15), 에너지드링크(3)" << std::endl;
    
    // 여러 자판기에서 오는 재고 조회 요청들
    std::vector<std::pair<std::string, std::string>> inquiries = {
        {"T1", "02"}, // T1에서 사이다 문의
        {"T3", "08"}, // T3에서 캔커피 문의
        {"T4", "01"}, // T4에서 콜라 문의
        {"T5", "09"}  // T5에서 물 문의
    };
    
    std::vector<network::Message> responses;
    
    for (const auto& inquiry : inquiries) {
        std::string requestingVm = inquiry.first;
        std::string drinkCode = inquiry.second;
        
        std::cout << "처리 중: " << requestingVm << "에서 음료 " << drinkCode << " 문의" << std::endl;
        
        // 재고 확인
        auto availability = inventoryService.checkDrinkAvailabilityAndPrice(drinkCode);
        
        // 응답 메시지 생성
        network::Message response;
        response.msg_type = network::Message::Type::RESP_STOCK;
        response.src_id = "T2";
        response.dst_id = requestingVm;
        response.msg_content["item_code"] = drinkCode;
        response.msg_content["item_num"] = std::to_string(availability.currentStock);
        response.msg_content["coor_x"] = "20";
        response.msg_content["coor_y"] = "20";
        
        responses.push_back(response);
        
        std::cout << "응답: " << drinkCode << " 재고 " << availability.currentStock << "개" << std::endl;
    }
    
    // 응답 검증
    EXPECT_EQ(responses.size(), 4);
    EXPECT_EQ(std::stoi(responses[0].msg_content.at("item_num")), 12); // 사이다
    EXPECT_EQ(std::stoi(responses[1].msg_content.at("item_num")), 8);  // 캔커피
    EXPECT_EQ(std::stoi(responses[2].msg_content.at("item_num")), 0);  // 콜라 (재고 없음)
    EXPECT_EQ(std::stoi(responses[3].msg_content.at("item_num")), 15); // 물
    
    std::cout << "✓ 테스트 4 완료: 여러 음료에 대한 연속 재고 조회 처리" << std::endl;
}

// 테스트 5: 잘못된 재고 조회 요청 처리 테스트
TEST(UC17Test, HandleInvalidStockInquiry) {
    std::cout << "=== UC17 테스트: 잘못된 재고 조회 요청 처리 ===" << std::endl;
    
    // Repository 및 Service 초기화
    persistence::InventoryRepository inventoryRepo;
    persistence::DrinkRepository drinkRepo;
    service::ErrorService errorService;
    service::InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    
    std::cout << "T3 자판기에서 잘못된 요청 처리 테스트" << std::endl;
    
    // 케이스 1: item_code 누락
    network::Message invalidRequest1;
    invalidRequest1.msg_type = network::Message::Type::REQ_STOCK;
    invalidRequest1.src_id = "T1";
    invalidRequest1.dst_id = "T3";
    // item_code가 없음
    invalidRequest1.msg_content["item_num"] = "1";
    
    bool processSuccess1 = false;
    try {
        std::string drinkCode = invalidRequest1.msg_content.at("item_code"); // 예외 발생 예상
        processSuccess1 = true;
    } catch (const std::exception& e) {
        std::cout << "잘못된 요청 1 (item_code 누락): 예외 발생 - " << e.what() << std::endl;
    }
    EXPECT_FALSE(processSuccess1);
    
    // 케이스 2: 존재하지 않는 음료 코드
    network::Message invalidRequest2;
    invalidRequest2.msg_type = network::Message::Type::REQ_STOCK;
    invalidRequest2.src_id = "T2";
    invalidRequest2.dst_id = "T3";
    invalidRequest2.msg_content["item_code"] = "99"; // 존재하지 않는 음료
    invalidRequest2.msg_content["item_num"] = "1";
    
    std::string invalidDrinkCode = invalidRequest2.msg_content.at("item_code");
    auto invalidAvailability = inventoryService.checkDrinkAvailabilityAndPrice(invalidDrinkCode);
    
    // 존재하지 않는 음료는 가격이 0이고 재고도 0
    EXPECT_EQ(invalidAvailability.price, 0);
    EXPECT_EQ(invalidAvailability.currentStock, 0);
    EXPECT_FALSE(invalidAvailability.isAvailable);
    
    std::cout << "존재하지 않는 음료 코드 '" << invalidDrinkCode << "': 가격 0원, 재고 0개" << std::endl;
    
    // 올바른 요청은 정상 처리되는지 확인
    network::Message validRequest;
    validRequest.msg_type = network::Message::Type::REQ_STOCK;
    validRequest.src_id = "T4";
    validRequest.dst_id = "T3";
    validRequest.msg_content["item_code"] = "01"; // 콜라
    validRequest.msg_content["item_num"] = "1";
    
    bool validProcessSuccess = false;
    try {
        std::string validDrinkCode = validRequest.msg_content.at("item_code");
        auto validAvailability = inventoryService.checkDrinkAvailabilityAndPrice(validDrinkCode);
        validProcessSuccess = true;
        std::cout << "올바른 요청: 콜라 재고 " << validAvailability.currentStock 
                  << "개, 가격 " << validAvailability.price << "원" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "올바른 요청 처리 중 예외: " << e.what() << std::endl;
    }
    
    EXPECT_TRUE(validProcessSuccess);
    std::cout << "✓ 테스트 5 완료: 잘못된 요청 거부, 올바른 요청 처리" << std::endl;
}
