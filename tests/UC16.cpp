#include <gtest/gtest.h>
#include "domain/order.h"
#include "domain/inventory.h"
#include "domain/drink.h"
#include "domain/vendingMachine.h"

#include "persistence/inventoryRepository.h"
#include "persistence/DrinkRepository.hpp"
#include "persistence/OrderRepository.hpp"

#include "service/ErrorService.hpp"
#include "service/InventoryService.hpp"
#include "service/DistanceService.hpp"
#include "network/message.hpp"

#include <memory>
#include <string>
#include <vector>

using domain::Order;
using domain::Inventory;
using domain::Drink;
using domain::VendingMachine;

// UC16: 재고 부족 시 다른 자판기에 재고 확보 요청 전송 테스트




// 테스트 1: 재고 확보 요청 메시지 생성 테스트
TEST(UC16Test, CreatePrepaymentReservationRequest) {
    std::cout << "=== UC16 테스트: 재고 확보 요청 메시지 생성 ===" << std::endl;
    
    // 선결제 주문 정보 (이미 생성되어 있다고 가정)
    std::string authCode = "ABC12";
    std::string drinkCode = "02"; // 사이다
    std::string targetVmId = "T2"; // 목표 자판기
    
    std::cout << "선결제 정보: 인증코드=" << authCode << ", 음료=" << drinkCode << ", 목표VM=" << targetVmId << std::endl;
    
    // 재고 확보 요청 메시지 생성
    network::Message reservationRequest;
    reservationRequest.msg_type = network::Message::Type::REQ_PREPAY;
    reservationRequest.src_id = "T1";
    reservationRequest.dst_id = targetVmId;
    reservationRequest.msg_content["item_code"] = drinkCode;
    reservationRequest.msg_content["item_num"] = "1";
    reservationRequest.msg_content["cert_code"] = authCode;
    
    // 메시지 내용 검증
    EXPECT_EQ(reservationRequest.msg_type, network::Message::Type::REQ_PREPAY);
    EXPECT_EQ(reservationRequest.src_id, "T1");
    EXPECT_EQ(reservationRequest.dst_id, "T2");
    EXPECT_EQ(reservationRequest.msg_content.at("item_code"), "02");
    EXPECT_EQ(reservationRequest.msg_content.at("item_num"), "1");
    EXPECT_EQ(reservationRequest.msg_content.at("cert_code"), "ABC12");
    
    std::cout << "재고 확보 요청 메시지: " << reservationRequest.src_id 
              << " -> " << reservationRequest.dst_id 
              << " (음료: " << reservationRequest.msg_content.at("item_code") << ")" << std::endl;
    std::cout << "✓ 테스트 1 완료: REQ_PREPAY 메시지 생성 성공" << std::endl;
}


// 테스트 3: 여러 자판기에 재고 확보 요청 테스트
TEST(UC16Test, SendMultiplePrepaymentRequests) {
    std::cout << "=== UC16 테스트: 여러 자판기에 재고 확보 요청 ===" << std::endl;
    
    // 공통 선결제 정보
    std::string authCode = "MULTI";
    std::string drinkCode = "01"; // 콜라
    
    // 여러 목표 자판기 목록
    std::vector<std::string> targetVms = {"T2", "T3", "T5"};
    
    std::cout << "선결제 정보: 인증코드=" << authCode << ", 음료=" << drinkCode << std::endl;
    std::cout << "목표 자판기 목록: ";
    for (const auto& vm : targetVms) {
        std::cout << vm << " ";
    }
    std::cout << std::endl;
    
    // 각 자판기에 요청 메시지 생성
    std::vector<network::Message> requests;
    for (const std::string& targetVm : targetVms) {
        network::Message request;
        request.msg_type = network::Message::Type::REQ_PREPAY;
        request.src_id = "T1";
        request.dst_id = targetVm;
        request.msg_content["item_code"] = drinkCode;
        request.msg_content["item_num"] = "1";
        request.msg_content["cert_code"] = authCode;
        
        requests.push_back(request);
        
        // 개별 메시지 검증
        EXPECT_EQ(request.dst_id, targetVm);
        EXPECT_EQ(request.msg_content.at("item_code"), drinkCode);
        
        std::cout << "요청 생성: T1 -> " << targetVm << " (콜라 1개)" << std::endl;
    }
    
    EXPECT_EQ(requests.size(), 3);
    std::cout << "✓ 테스트 3 완료: 여러 자판기에 대한 재고 확보 요청 생성" << std::endl;
}

