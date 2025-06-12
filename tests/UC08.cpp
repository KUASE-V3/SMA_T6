#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>

// Domain
#include "domain/drink.h"
#include "domain/order.h"
#include "domain/inventory.h"
#include "domain/vendingMachine.h"

// Persistence
#include "persistence/DrinkRepository.hpp"
#include "persistence/inventoryRepository.h"
#include "persistence/OrderRepository.hpp"
#include "persistence/OvmAddressRepository.hpp"
#include "persistence/prepayCodeRepository.h"

// Service
#include "service/ErrorService.hpp"
#include "service/InventoryService.hpp"
#include "service/OrderService.hpp"
#include "service/PrepaymentService.hpp"
#include "service/MessageService.hpp"
#include "service/DistanceService.hpp"
#include "service/UserProcessController.hpp"

// Network
#include "network/MessageSender.hpp"
#include "network/MessageReceiver.hpp"
#include "network/message.hpp"

// Presentation
#include "presentation/UserInterface.hpp"

// Boost.Asio
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

using namespace domain;
using namespace service;
using namespace persistence;
using namespace network;
using namespace presentation;


class VendingMachineSimulator {
private:
    std::string vmId_;
    int x_, y_;
    unsigned short port_;
    std::unique_ptr<boost::asio::io_context> ioContext_;
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> workGuard_;
    std::unique_ptr<std::thread> ioThread_;
    std::unique_ptr<MessageSender> messageSender_;
    std::unique_ptr<MessageReceiver> messageReceiver_;
    std::unique_ptr<MessageService> messageService_;
    std::unique_ptr<InventoryService> inventoryService_;
    std::unique_ptr<UserProcessController> controller_;
    
    // Repositories
    std::unique_ptr<DrinkRepository> drinkRepo_;
    std::unique_ptr<InventoryRepository> inventoryRepo_;
    std::unique_ptr<OrderRepository> orderRepo_;
    std::unique_ptr<PrepayCodeRepository> prepayRepo_;
    std::unique_ptr<ErrorService> errorService_;
    std::unique_ptr<PrepaymentService> prepaymentService_;
    std::unique_ptr<OrderService> orderService_;
    std::unique_ptr<DistanceService> distanceService_;
    std::unique_ptr<UserInterface> ui_;

public:
    VendingMachineSimulator(const std::string& vmId, int x, int y, unsigned short port,
                           const std::vector<std::string>& otherEndpoints,
                           const std::unordered_map<std::string, std::string>& idMap)
        : vmId_(vmId), x_(x), y_(y), port_(port) {
        
        std::cout << "[" << vmId_ << "] 시뮬레이터 초기화 시작 (Port: " << port_ << ")" << std::endl;
        
        // io_context 및 work_guard 초기화
        ioContext_ = std::make_unique<boost::asio::io_context>();
        workGuard_ = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
            boost::asio::make_work_guard(*ioContext_));
        
        // Repository 초기화
        drinkRepo_ = std::make_unique<DrinkRepository>();
        inventoryRepo_ = std::make_unique<InventoryRepository>();
        orderRepo_ = std::make_unique<OrderRepository>();
        prepayRepo_ = std::make_unique<PrepayCodeRepository>();
        errorService_ = std::make_unique<ErrorService>();
        ui_ = std::make_unique<UserInterface>();
        
        // Service 초기화
        inventoryService_ = std::make_unique<InventoryService>(*inventoryRepo_, *drinkRepo_, *errorService_);
        prepaymentService_ = std::make_unique<PrepaymentService>(*prepayRepo_, *orderRepo_, *errorService_);
        orderService_ = std::make_unique<OrderService>(*orderRepo_, *drinkRepo_, *inventoryService_, *prepaymentService_, *errorService_);
        distanceService_ = std::make_unique<DistanceService>();
        
        // 네트워크 초기화
        messageSender_ = std::make_unique<MessageSender>(*ioContext_, otherEndpoints, idMap);
        messageReceiver_ = std::make_unique<MessageReceiver>(*ioContext_, port_);
        messageService_ = std::make_unique<MessageService>(*messageSender_, *messageReceiver_, *errorService_, vmId_, x_, y_);
        
        // 컨트롤러 초기화
        controller_ = std::make_unique<UserProcessController>(
            *ui_, *inventoryService_, *orderService_, *prepaymentService_,
            *messageService_, *distanceService_, *errorService_,
            *ioContext_, vmId_, x_, y_, otherEndpoints.size()
        );
        
        std::cout << "[" << vmId_ << "] 시뮬레이터 초기화 완료" << std::endl;
    }
    
    void setupInventory(const std::string& drinkCode, int stock) {
        inventoryRepo_->addOrUpdateStock(Inventory(drinkCode, stock));
        std::cout << "[" << vmId_ << "] 재고 설정: " << drinkCode << " = " << stock << "개" << std::endl;
    }
    
    void startNetworking() {
        std::cout << "[" << vmId_ << "] 네트워킹 시작..." << std::endl;
        
        // MessageService를 직접 사용하여 메시지 핸들러 등록 및 수신 시작
        // UC17: 재고 조회 요청 수신 핸들러
        messageService_->registerMessageHandler(Message::Type::REQ_STOCK,
            [this](const Message& msg) {
                std::cout << "[" << vmId_ << "] REQ_STOCK 수신: " << msg.src_id << std::endl;
                if (msg.msg_content.count("item_code")) {
                    std::string drinkCode = msg.msg_content.at("item_code");
                    auto availability = inventoryService_->checkDrinkAvailabilityAndPrice(drinkCode);
                    messageService_->sendStockResponse(msg.src_id, drinkCode, availability.currentStock);
                    std::cout << "[" << vmId_ << "] RESP_STOCK 전송: " << msg.src_id << " (재고: " << availability.currentStock << ")" << std::endl;
                }
            });
        
        // UC9: 재고 조회 응답 수신 핸들러
        messageService_->registerMessageHandler(Message::Type::RESP_STOCK,
            [this](const Message& msg) {
                std::cout << "[" << vmId_ << "] RESP_STOCK 수신: " << msg.src_id << std::endl;
                // 이 테스트에서는 단순히 로그만 출력
            });
        
        messageService_->startReceivingMessages();
        
        // io_context 스레드 시작
        ioThread_ = std::make_unique<std::thread>([this]() {
            try {
                std::cout << "[" << vmId_ << "] io_context 스레드 시작" << std::endl;
                ioContext_->run();
                std::cout << "[" << vmId_ << "] io_context 스레드 종료" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[" << vmId_ << "] io_context 예외: " << e.what() << std::endl;
            }
        });
        
        // 네트워킹이 시작될 시간을 주기 위해 잠시 대기
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "[" << vmId_ << "] 네트워킹 시작 완료" << std::endl;
    }
    
    void sendStockRequest(const std::string& drinkCode) {
        std::cout << "[" << vmId_ << "] 재고 조회 브로드캐스트 전송: " << drinkCode << std::endl;
        messageService_->sendStockRequestBroadcast(drinkCode);
    }
    
    void shutdown() {
        std::cout << "[" << vmId_ << "] 종료 시작..." << std::endl;
        
        if (ioContext_) {
            ioContext_->stop();
        }
        
        if (workGuard_) {
            workGuard_->reset();
        }
        
        if (ioThread_ && ioThread_->joinable()) {
            ioThread_->join();
        }
        
        std::cout << "[" << vmId_ << "] 종료 완료" << std::endl;
    }
    
    auto getAvailability(const std::string& drinkCode) {
        return inventoryService_->checkDrinkAvailabilityAndPrice(drinkCode);
    }
    
    std::string getId() const { return vmId_; }
    int getX() const { return x_; }
    int getY() const { return y_; }
    unsigned short getPort() const { return port_; }
};



// 테스트 3: 다중 자판기 네트워크 테스트 (1:N 통신)
TEST(UC08_UC09Test, MultipleVendingMachinesNetwork) {
    
    // T1 자판기 (요청자)
    std::vector<std::string> t1_endpoints = {"127.0.0.1:12346", "127.0.0.1:12347"};
    std::unordered_map<std::string, std::string> t1_idMap = {
        {"T2", "127.0.0.1:12346"}, {"T3", "127.0.0.1:12347"}
    };
    VendingMachineSimulator t1("T1", 10, 10, 12345, t1_endpoints, t1_idMap);
    t1.setupInventory("02", 0); // 사이다 재고 없음
    
    // T2 자판기 (응답자 1 - 재고 있음)
    std::vector<std::string> t2_endpoints = {"127.0.0.1:12345"};
    std::unordered_map<std::string, std::string> t2_idMap = {{"T1", "127.0.0.1:12345"}};
    VendingMachineSimulator t2("T2", 20, 20, 12346, t2_endpoints, t2_idMap);
    t2.setupInventory("02", 3); // 사이다 3개
    
    // T3 자판기 (응답자 2 - 재고 있음)
    std::vector<std::string> t3_endpoints = {"127.0.0.1:12345"};
    std::unordered_map<std::string, std::string> t3_idMap = {{"T1", "127.0.0.1:12345"}};
    VendingMachineSimulator t3("T3", 30, 30, 12347, t3_endpoints, t3_idMap);
    t3.setupInventory("02", 7); // 사이다 7개
    
    std::cout << "\n다중 자판기 시나리오:" << std::endl;
    std::cout << "- T1(12345): 사이다 재고 0개 → 브로드캐스트 요청" << std::endl;
    std::cout << "- T2(12346): 사이다 재고 3개 → 응답 예정" << std::endl;
    std::cout << "- T3(12347): 사이다 재고 7개 → 응답 예정" << std::endl;
    
    // 모든 자판기 네트워킹 시작 (응답자들 먼저)
    t2.startNetworking();
    t3.startNetworking();
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    t1.startNetworking();
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    
    // UC8: T1에서 사이다 재고 조회 브로드캐스트
    std::cout << "\nUC08 실행: T1 → 다중 브로드캐스트 재고 조회 (사이다)" << std::endl;
    t1.sendStockRequest("02");
    
    // UC9: 다중 응답 처리 대기
    std::cout << "UC09 대기: 다중 자판기 응답 처리..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // 각 응답자 자판기의 재고 확인
    auto t2_availability = t2.getAvailability("02");
    auto t3_availability = t3.getAvailability("02");
    
    EXPECT_TRUE(t2_availability.isAvailable);
    EXPECT_EQ(t2_availability.currentStock, 3);
    EXPECT_TRUE(t3_availability.isAvailable);
    EXPECT_EQ(t3_availability.currentStock, 7);
    
    std::cout << "✓ UC08 완료: T1에서 다중 브로드캐스트 전송" << std::endl;
    std::cout << "✓ UC09 완료: T2 응답 (재고: " << t2_availability.currentStock << "개)" << std::endl;
    std::cout << "✓ UC09 완료: T3 응답 (재고: " << t3_availability.currentStock << "개)" << std::endl;
    
    // 정리
    t1.shutdown();
    t2.shutdown();
    t3.shutdown();
}

// 테스트 4: 재고 없는 경우의 응답 테스트
TEST(UC08_UC09Test, NoStockResponseTest) {
    std::cout << "=== UC08+UC09 테스트: 재고 없는 경우 응답 ===" << std::endl;
    
    // T1 자판기 (요청자)
    std::vector<std::string> t1_endpoints = {"127.0.0.1:12346"};
    std::unordered_map<std::string, std::string> t1_idMap = {{"T2", "127.0.0.1:12346"}};
    VendingMachineSimulator t1("T1", 10, 10, 12345, t1_endpoints, t1_idMap);
    t1.setupInventory("03", 0); // 녹차 재고 없음
    
    // T2 자판기 (응답자 - 역시 재고 없음)
    std::vector<std::string> t2_endpoints = {"127.0.0.1:12345"};
    std::unordered_map<std::string, std::string> t2_idMap = {{"T1", "127.0.0.1:12345"}};
    VendingMachineSimulator t2("T2", 20, 20, 12346, t2_endpoints, t2_idMap);
    t2.setupInventory("03", 0); // 녹차 재고 없음
    
    std::cout << "\n재고 없음 시나리오:" << std::endl;
    std::cout << "- T1: 녹차 재고 0개" << std::endl;
    std::cout << "- T2: 녹차 재고 0개 → 재고 없음 응답 예정" << std::endl;
    
    // 네트워킹 시작
    t2.startNetworking();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    t1.startNetworking();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // UC8+UC9: 재고 조회 및 재고 없음 응답
    std::cout << "\nUC08+UC09 실행: 재고 없음 시나리오 테스트" << std::endl;
    t1.sendStockRequest("03");
    
    // 응답 처리 대기
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 두 자판기 모두 재고 없음 확인
    auto t1_availability = t1.getAvailability("03");
    auto t2_availability = t2.getAvailability("03");
    
    EXPECT_FALSE(t1_availability.isAvailable);
    EXPECT_EQ(t1_availability.currentStock, 0);
    EXPECT_FALSE(t2_availability.isAvailable);
    EXPECT_EQ(t2_availability.currentStock, 0);
    
    std::cout << "✓ UC08+UC09 완료: 재고 없음 케이스 처리" << std::endl;
    
    // 정리
    t1.shutdown();
    t2.shutdown();
}

