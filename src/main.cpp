#include "domain/drink.h"
#include "domain/inventory.h"
#include "domain/order.h"
#include "domain/vendingMachine.h"
#include "domain/prepaymentCode.h"

#include "persistence/DrinkRepository.hpp"
#include "persistence/inventoryRepository.h"
#include "persistence/OrderRepository.hpp"
#include "persistence/OvmAddressRepository.hpp"
#include "persistence/prepayCodeRepository.h"

#include "network/message.hpp"
#include "network/MessageSender.hpp"
#include "network/MessageReceiver.hpp"

#include "service/ErrorService.hpp"
#include "service/InventoryService.hpp"
#include "service/DistanceService.hpp"
#include "service/PrepaymentService.hpp"
#include "service/OrderService.hpp"
#include "service/MessageService.hpp"
#include "service/UserProcessController.hpp"
#include "presentation/UserInterface.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <unordered_map>
#include <thread>
#include <algorithm>
#include <memory>
#include <set>

// --- 전체 시스템에 정의된 자판기 정보 ---
const std::vector<domain::VendingMachine> ALL_VENDING_MACHINES_IN_SYSTEM = {
    domain::VendingMachine("T1", 10, 10, "12345"), domain::VendingMachine("T2", 20, 20, "12346"),
    domain::VendingMachine("T3", 10, 30, "12347"), domain::VendingMachine("T4", 40, 10, "12348"),
    domain::VendingMachine("T5", 50, 50, "12349"), domain::VendingMachine("T6", 25, 40, "12350"),
    domain::VendingMachine("T7", 35, 25, "12351"), domain::VendingMachine("T8", 45, 35, "12352")
};

// --- 현재 실행될 자판기 정보 ---
std::string THIS_VM_ID = ""; // 명령행 인자로 설정
int THIS_VM_X = 0;
int THIS_VM_Y = 0;
unsigned short THIS_VM_PORT = 0;

// --- 고정된 테스트 대상 ---
const std::string PRIMARY_TEST_VM_ID = "T5";
const std::string SECONDARY_TEST_VM_ID = "T1";


// 함수 선언
domain::VendingMachine findVmDefinitionInList(const std::string& vmId, const std::vector<domain::VendingMachine>& vmList);
void setupFocusedInventory(const std::string& currentVmId, persistence::InventoryRepository& inventoryRepo);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "사용법: " << argv[0] << " [실행할 자판기 ID (예: T1 또는 T5)]" << std::endl;
        std::cerr << "이 테스트 빌드에서는 T1과 T5 간의 통신에 집중합니다." << std::endl;
        return 1;
    }
    THIS_VM_ID = argv[1];

    if (THIS_VM_ID != PRIMARY_TEST_VM_ID && THIS_VM_ID != SECONDARY_TEST_VM_ID) {
        std::cerr << "오류: 이 테스트 빌드에서는 " << PRIMARY_TEST_VM_ID << " 또는 "
                  << SECONDARY_TEST_VM_ID << "만 실행할 수 있습니다." << std::endl;
        return 1;
    }

    domain::VendingMachine vmDef = findVmDefinitionInList(THIS_VM_ID, ALL_VENDING_MACHINES_IN_SYSTEM);
    if (vmDef.getId().empty()) {
        std::cerr << "오류: ID(" << THIS_VM_ID << ")에 대한 정의를 ALL_VENDING_MACHINES_IN_SYSTEM에서 찾을 수 없음. 종료." << std::endl;
        return 1;
    }
    THIS_VM_X = vmDef.getLocation().first;
    THIS_VM_Y = vmDef.getLocation().second;
    try {
        THIS_VM_PORT = static_cast<unsigned short>(std::stoi(vmDef.getPort()));
    } catch (const std::exception& e) {
        std::cerr << "오류: ID(" << THIS_VM_ID << ")의 포트(" << vmDef.getPort() << ") 변환 실패. 종료. " << e.what() << std::endl;
        return 1;
    }

    std::cout << "--- 자판기 " << THIS_VM_ID << " (Port: " << THIS_VM_PORT << ", X: " << THIS_VM_X << ", Y: " << THIS_VM_Y << ") 테스트 시작 ---" << std::endl;
    std::cout << "정보: 이 실행은 " << PRIMARY_TEST_VM_ID << "과 " << SECONDARY_TEST_VM_ID << " 간의 통신에 집중합니다." << std::endl;

    try {
        boost::asio::io_context io_context;
        auto work_guard = boost::asio::make_work_guard(io_context);

        presentation::UserInterface ui;
        persistence::DrinkRepository drinkRepository;
        persistence::InventoryRepository inventoryRepository;
        persistence::OrderRepository orderRepository;
        persistence::OvmAddressRepository ovmAddressRepository;
        persistence::PrepayCodeRepository prepayCodeRepository;

        setupFocusedInventory(THIS_VM_ID, inventoryRepository);
        std::cout << "정보: " << THIS_VM_ID << " 자판기의 테스트용 초기 재고 설정 완료." << std::endl;

        std::vector<std::string> other_vm_endpoints_for_sender;
        std::unordered_map<std::string, std::string> id_to_endpoint_map_for_sender;
        int actual_other_vm_count = 0;

        // 고정된 상대방 VM 정보 설정
        std::string targetPartnerVmId = (THIS_VM_ID == PRIMARY_TEST_VM_ID) ? SECONDARY_TEST_VM_ID : PRIMARY_TEST_VM_ID;
        domain::VendingMachine partnerVmDef = findVmDefinitionInList(targetPartnerVmId, ALL_VENDING_MACHINES_IN_SYSTEM);

        if (!partnerVmDef.getId().empty()) {
            ovmAddressRepository.save(partnerVmDef);
            std::string endpoint_str = "127.0.0.1:" + partnerVmDef.getPort();
            other_vm_endpoints_for_sender.push_back(endpoint_str); // 브로드캐스트 시 이 하나만 대상
            id_to_endpoint_map_for_sender[partnerVmDef.getId()] = endpoint_str;
            actual_other_vm_count = 1; // 통신 대상은 고정된 상대방 1대
            std::cout << "정보: 통신 대상 자판기 " << partnerVmDef.getId() << " (Port: " << partnerVmDef.getPort() << ", Endpoint: " << endpoint_str << ")로 설정됨." << std::endl;
        } else {
            std::cerr << "경고: 파트너 자판기 " << targetPartnerVmId << " 정보를 찾을 수 없어 다른 자판기 통신이 불가능합니다." << std::endl;
        }

        network::MessageSender messageSender(io_context, other_vm_endpoints_for_sender, id_to_endpoint_map_for_sender);
        network::MessageReceiver messageReceiver(io_context, THIS_VM_PORT);

        service::ErrorService errorService;
        service::InventoryService inventoryService(inventoryRepository, drinkRepository, errorService);
        service::DistanceService distanceService;
        service::PrepaymentService prepaymentService(prepayCodeRepository, orderRepository, errorService);
        service::MessageService messageService(messageSender, messageReceiver, errorService, THIS_VM_ID, THIS_VM_X, THIS_VM_Y);
        service::OrderService orderService(orderRepository, drinkRepository, inventoryService, prepaymentService, errorService);

        service::UserProcessController controller(
            ui, inventoryService, orderService, prepaymentService,
            messageService, distanceService, errorService,
            io_context, THIS_VM_ID, THIS_VM_X, THIS_VM_Y,
            actual_other_vm_count
        );

        std::thread io_thread([&io_context, vm_id = THIS_VM_ID](){
            try {
                std::cout << "정보: " << vm_id << "의 io_context 스레드 시작." << std::endl;
                io_context.run();
                std::cout << "정보: " << vm_id << "의 io_context 스레드 정상 종료." << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "오류: " << vm_id << "의 io_context 스레드에서 예외 발생: " << e.what() << std::endl;
            }
        });

        controller.run();

        std::cout << "정보: " << THIS_VM_ID << " 자판기 메인 루프 종료. 시스템 종료 절차 시작..." << std::endl;
        io_context.stop();
        work_guard.reset();
        if (io_thread.joinable()) {
            std::cout << "정보: " << THIS_VM_ID << " io_context 스레드 조인 대기..." << std::endl;
            io_thread.join();
        }
        std::cout << "정보: " << THIS_VM_ID << " io_context 스레드 조인 완료." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "자판기 " << THIS_VM_ID << " 초기화 또는 실행 중 심각한 오류 발생: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "자판기 " << THIS_VM_ID << "에서 알 수 없는 심각한 오류 발생." << std::endl;
        return 1;
    }

    std::cout << "--- 자판기 " << THIS_VM_ID << " 시스템 정상 종료 ---" << std::endl;
    return 0;
}

domain::VendingMachine findVmDefinitionInList(const std::string& vmId, const std::vector<domain::VendingMachine>& vmList) {
    auto it = std::find_if(vmList.begin(), vmList.end(),
                           [&vmId](const domain::VendingMachine& vm){ return vm.getId() == vmId; });
    if (it != vmList.end()) return *it;
    return domain::VendingMachine();
}

void setupFocusedInventory(const std::string& currentVmId, persistence::InventoryRepository& inventoryRepo) {
    std::cout << "정보: " << currentVmId << " 자판기의 테스트용 재고를 설정합니다." << std::endl;
    if (currentVmId == PRIMARY_TEST_VM_ID) { // T5
        inventoryRepo.addOrUpdateStock(domain::Inventory("01", 0));  // 콜라 (T5에는 없음)
        inventoryRepo.addOrUpdateStock(domain::Inventory("02", 5));  // 사이다 (T5에는 있음)
        std::cout << "  " << PRIMARY_TEST_VM_ID << ": 콜라(0), 사이다(5) 설정됨." << std::endl;
    } else if (currentVmId == SECONDARY_TEST_VM_ID) { // T1
        inventoryRepo.addOrUpdateStock(domain::Inventory("01", 10)); // 콜라 (T1에는 있음)
        inventoryRepo.addOrUpdateStock(domain::Inventory("02", 0));  // 사이다 (T1에는 없음)
        std::cout << "  " << SECONDARY_TEST_VM_ID << ": 콜라(10), 사이다(0) 설정됨." << std::endl;
    }
}