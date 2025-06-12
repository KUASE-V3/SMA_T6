#include <gtest/gtest.h>
#include "domain/order.h"
#include "domain/inventory.h"
#include "domain/prepaymentCode.h"

#include "persistence/inventoryRepository.h"
#include "persistence/DrinkRepository.hpp"
#include "persistence/prepayCodeRepository.h"
#include "persistence/OrderRepository.hpp"

#include "service/ErrorService.hpp"
#include "service/InventoryService.hpp"
#include "service/PrepaymentService.hpp"
#include "network/message.hpp"
#include "service/MessageService.hpp"
#include "network/MessageSender.hpp"
#include "network/MessageReceiver.hpp"

#include <boost/asio/io_context.hpp>
#include <memory>
#include <string>

using domain::Order;
using domain::Inventory;
using domain::PrePaymentCode;

// UC15: 선결제 요청 수신 및 재고 확보 테스트

// 테스트 1: 재고가 충분한 경우 선결제 요청 수락 테스트
TEST(UC15Test, AcceptPrepaymentRequestWithSufficientStock) {
    boost::asio::io_context io_context;
    std::vector<std::string> empty_endpoints;
    std::unordered_map<std::string, std::string> empty_id_map;
    
    // Repository 및 Service 초기화
    persistence::InventoryRepository inventoryRepo;
    persistence::DrinkRepository drinkRepo;
    persistence::PrepayCodeRepository prepayRepo;
    persistence::OrderRepository orderRepo;
    service::ErrorService errorService;
    service::InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    service::PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    
    network::MessageSender messageSender(io_context, empty_endpoints, empty_id_map);
    network::MessageReceiver messageReceiver(io_context, 12345);
    service::MessageService messageService(messageSender, messageReceiver, errorService, "T2", 20, 20);
    
    // 초기 재고 설정 (T2 자판기: 사이다 12개)
    inventoryRepo.addOrUpdateStock(Inventory("02", 12)); // 사이다 12개
    
    // 사용 전 재고 확인
    auto beforeStock = inventoryService.checkDrinkAvailabilityAndPrice("02");
    EXPECT_TRUE(beforeStock.isAvailable);
    EXPECT_EQ(beforeStock.currentStock, 12);
    
    std::cout << "T2 자판기 초기 상태: 사이다 " << beforeStock.currentStock << "개" << std::endl;
    
    // T1에서 보낸 선결제 요청 메시지 시뮬레이션
    network::Message prepayRequest;
    prepayRequest.msg_type = network::Message::Type::REQ_PREPAY;
    prepayRequest.src_id = "T1";
    prepayRequest.dst_id = "T2";
    prepayRequest.msg_content["item_code"] = "02"; // 사이다
    prepayRequest.msg_content["item_num"] = "1";
    prepayRequest.msg_content["cert_code"] = "ABC12";
    
    std::cout << "T1에서 T2로 선결제 요청: 사이다(02), 인증코드 ABC12" << std::endl;
    
    // 재고 확인 및 차감
    auto availability = inventoryService.checkDrinkAvailabilityAndPrice("02");
    bool reservationSuccess = false;
    
    if (availability.isAvailable && availability.currentStock >= 1) {
        inventoryService.decreaseStock("02"); // 재고 1개 차감
        prepaymentService.recordIncomingPrepayment("ABC12", "02", "T1");
        reservationSuccess = true;
        std::cout << "재고 확보 성공: 사이다 1개 예약, 재고 차감" << std::endl;
    }
    
    EXPECT_TRUE(reservationSuccess);
    
    // 재고 차감 확인
    auto afterStock = inventoryService.checkDrinkAvailabilityAndPrice("02");
    EXPECT_EQ(afterStock.currentStock, 11); // 12 - 1 = 11
    
    // 선결제 정보 저장 확인
    PrePaymentCode savedPrepay = prepayRepo.findByCode("ABC12");
    EXPECT_FALSE(savedPrepay.getCode().empty());
    EXPECT_EQ(savedPrepay.getCode(), "ABC12");
    
    auto savedOrder = savedPrepay.getHeldOrder();
    ASSERT_NE(savedOrder, nullptr);
    EXPECT_EQ(savedOrder->getDrinkCode(), "02");
    EXPECT_EQ(savedOrder->getVmid(), "T1"); // 요청한 자판기 ID
    
    std::cout << "✓ 테스트 1 완료: 충분한 재고로 선결제 요청 수락, 재고 차감 및 선결제 정보 저장" << std::endl;
}

// 테스트 2: 재고가 부족한 경우 선결제 요청 거절 테스트
TEST(UC15Test, RejectPrepaymentRequestWithInsufficientStock) {
    boost::asio::io_context io_context;
    std::vector<std::string> empty_endpoints;
    std::unordered_map<std::string, std::string> empty_id_map;
    
    // Repository 및 Service 초기화
    persistence::InventoryRepository inventoryRepo;
    persistence::DrinkRepository drinkRepo;
    persistence::PrepayCodeRepository prepayRepo;
    persistence::OrderRepository orderRepo;
    service::ErrorService errorService;
    service::InventoryService inventoryService(inventoryRepo, drinkRepo, errorService);
    service::PrepaymentService prepaymentService(prepayRepo, orderRepo, errorService);
    
    network::MessageSender messageSender(io_context, empty_endpoints, empty_id_map);
    network::MessageReceiver messageReceiver(io_context, 12346);
    service::MessageService messageService(messageSender, messageReceiver, errorService, "T1", 10, 10);
    
    // 재고 없음 설정 (T1 자판기: 사이다 0개)
    inventoryRepo.addOrUpdateStock(Inventory("02", 0)); // 사이다 재고 없음
    
    // 사용 전 재고 확인
    auto beforeStock = inventoryService.checkDrinkAvailabilityAndPrice("02");
    EXPECT_FALSE(beforeStock.isAvailable);
    EXPECT_EQ(beforeStock.currentStock, 0);
    
    std::cout << "T1 자판기 초기 상태: 사이다 재고 없음 (0개)" << std::endl;
    
    // T3에서 보낸 선결제 요청 메시지 시뮬레이션
    network::Message prepayRequest;
    prepayRequest.msg_type = network::Message::Type::REQ_PREPAY;
    prepayRequest.src_id = "T3";
    prepayRequest.dst_id = "T1";
    prepayRequest.msg_content["item_code"] = "02"; // 사이다
    prepayRequest.msg_content["item_num"] = "1";
    prepayRequest.msg_content["cert_code"] = "XYZ99";
    
    std::cout << "T3에서 T1로 선결제 요청: 사이다(02), 인증코드 XYZ99" << std::endl;
    
    // 재고 확인 및 처리
    auto availability = inventoryService.checkDrinkAvailabilityAndPrice("02");
    bool reservationSuccess = false;
    
    if (availability.isAvailable && availability.currentStock >= 1) {
        inventoryService.decreaseStock("02");
        prepaymentService.recordIncomingPrepayment("XYZ99", "02", "T3");
        reservationSuccess = true;
    } else {
        std::cout << "재고 부족으로 선결제 요청 거절" << std::endl;
    }
    
    EXPECT_FALSE(reservationSuccess);
    
    // 재고가 변경되지 않았는지 확인
    auto afterStock = inventoryService.checkDrinkAvailabilityAndPrice("02");
    EXPECT_EQ(afterStock.currentStock, 0); // 여전히 0개
    
    // 선결제 정보가 저장되지 않았는지 확인
    PrePaymentCode savedPrepay = prepayRepo.findByCode("XYZ99");
    EXPECT_TRUE(savedPrepay.getCode().empty()); // 저장되지 않음
    
    std::cout << "✓ 테스트 2 완료: 재고 부족으로 선결제 요청 거절, 재고 및 데이터 변경 없음" << std::endl;
}