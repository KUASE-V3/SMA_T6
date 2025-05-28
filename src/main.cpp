#include "domain/drink.h" // 도메인 객체 (경로 및 확장자 확인 필요)
#include "persistence/DrinkRepository.hpp"
#include "persistence/inventoryRepository.h"
#include "persistence/OrderRepository.hpp"
#include "persistence/OvmAddressRepository.hpp"
#include "persistence/prepayCodeRepository.h"
#include "network/message.hpp"
#include "network/MessageSender.hpp"
#include "network/MessageReceiver.hpp"
#include "network/PaymentCallbackReceiver.hpp" // UserProcessController에서 로컬로 사용하지만, 의존성으로 뺄 수도 있음
#include "service/ErrorService.hpp"
#include "service/InventoryService.hpp"
#include "service/DistanceService.hpp"
#include "service/PrepaymentService.hpp"
#include "service/OrderService.hpp"
#include "service/MessageService.hpp"
#include "service/UserProcessController.hpp"
#include "presentation/UserInterface.hpp"

#include <boost/asio/io_context.hpp>
#include <vector>
#include <string>
#include <iostream>

// 현재 자판기 정보 (설정 파일이나 명령줄 인자로부터 받아올 수 있음)
const std::string MY_VM_ID = "T1"; // 예시 ID
const int MY_VM_X = 10;
const int MY_VM_Y = 10;
const unsigned short MY_VM_PORT = 12345; // 예시 포트

// 다른 자판기들 정보 (설정 파일 등에서 로드)
// MessageSender와 OvmAddressRepository 초기화에 사용됨
const std::vector<std::string> OTHER_VM_ENDPOINTS = {
    // "host1:port1", "host2:port2", ...
    // 테스트를 위해 루프백 주소와 다른 포트 사용 가능
    // 예: "127.0.0.1:12346", "127.0.0.1:12347"
};
const std::map<std::string, std::string> OTHER_VM_ID_TO_ENDPOINT_MAP = {
    // {"T2", "127.0.0.1:12346"}, {"T3", "127.0.0.1:12347"}, ...
};
const std::vector<domain::VendingMachine> OTHER_VM_DEFINITIONS = {
    // domain::VendingMachine("T2", 20, 20, "12346"),
    // domain::VendingMachine("T3", 30, 10, "12347"),
    // ...
};


int main() {
    try {
        // 0. Boost.Asio io_context 생성 (네트워크 및 비동기 작업용)
        boost::asio::io_context io_context;

        // 1. Presentation Layer 객체 생성
        presentation::UserInterface ui;

        // 2. Persistence Layer (Repository) 객체 생성
        persistence::DrinkRepository drinkRepository;
        persistence::InventoryRepository inventoryRepository;
        // --- 초기 재고 설정 (예시) ---
        // 실제로는 파일이나 데이터베이스에서 로드하거나, 관리자 기능으로 설정
        inventoryRepository.addOrUpdateStock(domain::Inventory("01", 10)); // 콜라 10개
        inventoryRepository.addOrUpdateStock(domain::Inventory("02", 5));  // 사이다 5개
        // ... (자판기당 최대 7종류 음료 [cite: 74])

        persistence::OrderRepository orderRepository;
        persistence::OvmAddressRepository ovmAddressRepository;
        // --- 다른 자판기 정보 등록 (예시) ---
        for(const auto& vm_def : OTHER_VM_DEFINITIONS) {
            ovmAddressRepository.save(vm_def);
        }

        persistence::PrepayCodeRepository prepayCodeRepository;

        // 3. Network Layer 객체 생성
        // MessageSender를 위한 endpoint 목록과 ID-endpoint 맵 구성
        std::vector<std::string> all_other_vm_endpoints_for_sender;
        std::unordered_map<std::string, std::string> id_to_endpoint_map_for_sender;

        for(const auto& vm_def : OTHER_VM_DEFINITIONS){
            // 엔드포인트 형식은 "호스트:포트" 문자열
            // 여기서는 VendingMachine의 getPort가 포트 번호만 반환한다고 가정하고, 호스트는 로컬호스트로 임시 설정
            std::string endpoint_str = "127.0.0.1:" + vm_def.getPort();
            all_other_vm_endpoints_for_sender.push_back(endpoint_str);
            id_to_endpoint_map_for_sender[vm_def.getId()] = endpoint_str;
        }
        // 또는 OTHER_VM_ENDPOINTS 와 OTHER_VM_ID_TO_ENDPOINT_MAP 상수를 직접 사용

        network::MessageSender messageSender(io_context, all_other_vm_endpoints_for_sender, id_to_endpoint_map_for_sender);
        network::MessageReceiver messageReceiver(io_context, MY_VM_PORT);
        // network::PaymentCallbackReceiver는 UserProcessController에서 로컬로 사용됨 (필요시 주입)

        // 4. Service Layer 객체 생성 (의존성 주입)
        service::ErrorService errorService;
        service::InventoryService inventoryService(inventoryRepository, drinkRepository, errorService);
        service::DistanceService distanceService; // 생성자에 ErrorService 주입 안 함 (헤더 기준)
        service::PrepaymentService prepaymentService(prepayCodeRepository, orderRepository, errorService);
        service::MessageService messageService(messageSender, messageReceiver, errorService, MY_VM_ID, MY_VM_X, MY_VM_Y); // 수정된 호출 방식
        service::OrderService orderService(orderRepository, drinkRepository, inventoryService, prepaymentService, messageService, errorService);
        
        service::UserProcessController controller(
            ui,
            inventoryService,
            orderService,
            prepaymentService,
            messageService,
            distanceService,
            errorService,
            io_context,
            MY_VM_ID,
            MY_VM_X,
            MY_VM_Y
        );

        // 5. UserProcessController 실행
        controller.run();

        // 만약 io_context.run()을 별도 스레드에서 돌린다면 여기서 join 필요
        // 현재는 controller.run() 내에서 poll_one() 등으로 처리하므로, run() 종료 시 프로그램 종료.

    } catch (const std::exception& e) {
        std::cerr << "시스템 전체 오류 발생: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "알 수 없는 시스템 전체 오류 발생." << std::endl;
        return 1;
    }

    return 0;
}