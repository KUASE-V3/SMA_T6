#include "domain/drink.h"
#include "domain/inventory.h"
#include "domain/order.h"
#include "domain/vendingMachine.h"
#include "domain/prepaymentCode.h"

#include "persistence/DrinkRepository.hpp"
#include "persistence/inventoryRepository.h"    // .h 또는 .hpp 프로젝트에 맞게 확인
#include "persistence/OrderRepository.hpp"
#include "persistence/OvmAddressRepository.hpp"
#include "persistence/prepayCodeRepository.h" // .h 또는 .hpp 프로젝트에 맞게 확인

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

// --- 현재 실행될 자판기의 정보 (명령행 인자로 덮어쓰일 수 있음) ---
std::string THIS_VM_ID = "T6"; // T6
int THIS_VM_X = 50;             // T6의 기본 X 좌표
int THIS_VM_Y = 50;            // T6의 기본 Y 좌표
unsigned short THIS_VM_PORT = 12350; // T6의 기본 포트

// 함수 선언
domain::VendingMachine findVmDefinitionInList(const std::string& vmId, const std::vector<domain::VendingMachine>& vmList);
void setupGeneralInventory(const std::string& currentVmId, persistence::InventoryRepository& inventoryRepo);

int main(int argc, char* argv[]) {


    // 1. 명령행 인자 파싱
    if (argc >= 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        std::cout << "사용법: " << argv[0] << " [자판기ID [X Y Port | Port]]" << std::endl;
        std::cout << "  옵션 없음        : 기본 자판기(" << THIS_VM_ID << ")가 기본 설정으로 실행됩니다." << std::endl;
        std::cout << "  [Port]           : 기본 자판기(" << THIS_VM_ID << ")가 지정된 포트로 실행됩니다." << std::endl;
        std::cout << "  [자판기ID]       : 지정된 ID의 자판기가 기본 설정으로 실행됩니다." << std::endl;
        std::cout << "  [자판기ID Port]  : 지정된 ID의 자판기가 지정된 포트로 실행됩니다 (좌표는 기본값 사용)." << std::endl;
        std::cout << "  [자판기ID X Y Port]: 지정된 ID의 자판기가 지정된 X, Y 좌표 및 포트로 실행됩니다." << std::endl;
        std::cout << "\nALL_VENDING_MACHINES_IN_SYSTEM 정의:" << std::endl;
        for(const auto& vm : ALL_VENDING_MACHINES_IN_SYSTEM) {
            std::cout << "  - ID: " << vm.getId() << ", X: " << vm.getLocation().first
                      << ", Y: " << vm.getLocation().second << ", Port: " << vm.getPort() << std::endl;
        }
        return 0;
    }

    std::string initialVmId = THIS_VM_ID; // 파싱 전 기본 ID (T6)
    unsigned short initialVmPort = THIS_VM_PORT;
    int initialVmX = THIS_VM_X;
    int initialVmY = THIS_VM_Y;

    if (argc == 2) { // 인자 1개: Port (for T6) 또는 ID
        try {
            initialVmPort = static_cast<unsigned short>(std::stoi(argv[1]));
            // 숫자로 변환 성공 시, 기본 ID(T6)에 대한 포트 변경으로 간주
            std::cout << "정보: 기본 자판기(" << initialVmId << ")의 실행 포트가 " << initialVmPort << "로 지정되었습니다." << std::endl;
        } catch (const std::invalid_argument& ia) { // 숫자가 아니면 ID로 간주
            initialVmId = argv[1];
            std::cout << "정보: 실행 자판기 ID가 " << initialVmId << "로 지정되었습니다. (기본 설정 사용)" << std::endl;
        } catch (const std::out_of_range& oor) {
            std::cerr << "오류: 포트 번호(" << argv[1] << ")가 유효한 범위를 벗어났습니다." << std::endl; return 1;
        }
    } else if (argc == 3) { // 인자 2개: ID Port
        initialVmId = argv[1];
        try {
            initialVmPort = static_cast<unsigned short>(std::stoi(argv[2]));
            std::cout << "정보: 자판기 " << initialVmId << "의 실행 포트가 " << initialVmPort << "로 지정되었습니다." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "오류: 포트 번호(" << argv[2] << ") 변환 실패. " << e.what() << std::endl; return 1;
        }
    } else if (argc == 5) { // 인자 4개: ID X Y Port
        initialVmId = argv[1];
        try {
            initialVmX = std::stoi(argv[2]);
            initialVmY = std::stoi(argv[3]);
            initialVmPort = static_cast<unsigned short>(std::stoi(argv[4]));
            std::cout << "정보: 자판기 " << initialVmId << "가 X:" << initialVmX << ", Y:" << initialVmY << ", Port:" << initialVmPort << "로 실행됩니다." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "오류: 제공된 실행 인자 (X, Y, Port) 변환 실패. " << e.what() << std::endl; return 1;
        }
    } else if (argc != 1) { // 그 외 잘못된 인자 수 (1개도 아니고, 2개도 아니고, 3개도 아니고, 5개도 아님)
        std::cerr << "오류: 잘못된 수의 명령행 인자입니다. 도움말은 --help 또는 -h 옵션을 사용하세요." << std::endl;
        return 1;
    }

    // 최종 VM 정보 설정
    THIS_VM_ID = initialVmId;
    domain::VendingMachine vmDef = findVmDefinitionInList(THIS_VM_ID, ALL_VENDING_MACHINES_IN_SYSTEM);
    if (vmDef.getId().empty()) {
        std::cerr << "오류: ID(" << THIS_VM_ID << ")에 대한 정의를 ALL_VENDING_MACHINES_IN_SYSTEM에서 찾을 수 없음. 종료." << std::endl;
        return 1;
    }

    if (argc < 5) { // X, Y가 인자로 주어지지 않은 경우, vmDef에서 가져옴
        THIS_VM_X = vmDef.getLocation().first;
        THIS_VM_Y = vmDef.getLocation().second;
    }
    if (argc == 1 || (argc == 2 && !isdigit(argv[1][0])) || argc ==3 ) { // Port가 인자로 명시적으로 주어지지 않은 경우(ID만 주어졌거나 아무것도 안주어졌거나), vmDef에서 가져옴
        // argc == 3 (ID Port)는 위에서 initialVmPort로 이미 설정됨. 여기서는 덮어쓰지 않도록 조건 조정.
        if (! (argc == 3 || (argc == 2 && isdigit(argv[1][0])) ) ) { // ID와 Port가 같이 주어졌거나, Port만 주어졌을때는 이미 initialVmPort 설정됨
            try { THIS_VM_PORT = static_cast<unsigned short>(std::stoi(vmDef.getPort())); }
            catch (const std::exception& e) { std::cerr << "오류: ID(" << THIS_VM_ID << ")의 기본 포트(" << vmDef.getPort() << ") 변환 실패. " << e.what() << std::endl; return 1; }
        } else {
            THIS_VM_PORT = initialVmPort; // 인자로 받은 포트 사용
        }
    } else if (argc == 5) { // ID X Y Port 모두 주어진 경우
         THIS_VM_X = initialVmX;
         THIS_VM_Y = initialVmY;
         THIS_VM_PORT = initialVmPort;
    } else { //argc == 2 이고 argv[1]이 port인 경우
        THIS_VM_PORT = initialVmPort;
    }


    std::cout << "--- 자판기 " << THIS_VM_ID << " (Port: " << THIS_VM_PORT << ", X: " << THIS_VM_X << ", Y: " << THIS_VM_Y << ") 시스템 시작 ---" << std::endl;

    try {
        boost::asio::io_context io_context;
        auto work_guard = boost::asio::make_work_guard(io_context);

        presentation::UserInterface ui;
        persistence::DrinkRepository drinkRepository;
        persistence::InventoryRepository inventoryRepository;
        persistence::OrderRepository orderRepository;
        persistence::OvmAddressRepository ovmAddressRepository;
        persistence::PrepayCodeRepository prepayCodeRepository;

        setupGeneralInventory(THIS_VM_ID, inventoryRepository);
        std::cout << "정보: " << THIS_VM_ID << " 자판기의 초기 재고 설정 완료." << std::endl;

        std::vector<std::string> other_vm_endpoints_for_sender;
        std::unordered_map<std::string, std::string> id_to_endpoint_map_for_sender;
        int actual_other_vm_count = 0;

        std::cout << "정보: 다른 자판기 정보 로드 중 (컨테이너 이름 기반)..." << std::endl;
        for (const auto& other_vm_def : ALL_VENDING_MACHINES_IN_SYSTEM) {
            if (other_vm_def.getId() == THIS_VM_ID) {
                continue; // 자기 자신은 제외
            }

            // Docker 네트워크 내에서는 컨테이너 이름으로 통신 가능
            // 컨테이너 이름을 자판기 ID를 기반으로 생성 (예: T1 -> vm_t1)
            // docker run 시 --name 옵션으로 지정한 이름과 일치해야 함.
            std::string container_name = "vm_" + other_vm_def.getId(); // 또는 "vm_t1", "vm_t5" 등 일관된 규칙 사용
            std::string endpoint_str = container_name + ":" + other_vm_def.getPort(); // 예: "vm_t1:12345"

            other_vm_endpoints_for_sender.push_back(endpoint_str);
            id_to_endpoint_map_for_sender[other_vm_def.getId()] = endpoint_str;
            actual_other_vm_count++;
            std::cout << "  + " << other_vm_def.getId() << " (컨테이너명: " << container_name 
                      << ", 포트: " << other_vm_def.getPort() << ", 내부 엔드포인트: " << endpoint_str 
                      << ") 통신 대상으로 추가됨." << std::endl;
        }
        if (actual_other_vm_count == 0 && ALL_VENDING_MACHINES_IN_SYSTEM.size() > 1) {
             std::cout << "경고: 다른 자판기가 시스템에 정의되어 있으나, 통신 대상으로 설정된 다른 자판기가 없습니다 (자기 자신만 시스템에 있는 경우)." << std::endl;
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

void setupGeneralInventory(const std::string& currentVmId, persistence::InventoryRepository& inventoryRepo) {
    std::cout << "정보: " << currentVmId << " 자판기의 일반 재고를 설정합니다." << std::endl;
    if (currentVmId == "T6") { // 우리 조 자판기 (T6)
        inventoryRepo.addOrUpdateStock(domain::Inventory("01", 0));  // 콜라 (선결제 테스트용으로 재고 없음)
        inventoryRepo.addOrUpdateStock(domain::Inventory("02", 5));  // 사이다 (로컬 구매 가능)
        inventoryRepo.addOrUpdateStock(domain::Inventory("03", 10)); // 녹차
        inventoryRepo.addOrUpdateStock(domain::Inventory("06", 0));  // 탄산수 (선결제 테스트용으로 재고 없음)
        inventoryRepo.addOrUpdateStock(domain::Inventory("08", 7));  // 캔커피
        inventoryRepo.addOrUpdateStock(domain::Inventory("15", 3));  // 오렌지주스
        inventoryRepo.addOrUpdateStock(domain::Inventory("20", 4));  // 카페라떼
        std::cout << "  T6: 콜라(0), 사이다(5), 녹차(10), 탄산수(0), 캔커피(7), 오렌지주스(3), 카페라떼(4) 설정됨." << std::endl;
    } else if (currentVmId == "T1") { // T6가 선결제할 대상 자판기 예시
        inventoryRepo.addOrUpdateStock(domain::Inventory("01", 10)); // 콜라 (T6가 선결제 가능)
        inventoryRepo.addOrUpdateStock(domain::Inventory("02", 0));
        inventoryRepo.addOrUpdateStock(domain::Inventory("04", 8));  // 홍차
        inventoryRepo.addOrUpdateStock(domain::Inventory("09", 15)); // 물
        inventoryRepo.addOrUpdateStock(domain::Inventory("10", 3));
        inventoryRepo.addOrUpdateStock(domain::Inventory("11", 6));
        inventoryRepo.addOrUpdateStock(domain::Inventory("12", 9));
        std::cout << "  T1: 콜라(10), 사이다(0), 홍차(8), 물(15), 에너지드링크(3), 유자차(6), 식혜(9) 설정됨." << std::endl;
    } else if (currentVmId == "T2") {
        inventoryRepo.addOrUpdateStock(domain::Inventory("02", 12)); // 사이다
        inventoryRepo.addOrUpdateStock(domain::Inventory("06", 10)); // 탄산수 (T6가 선결제 가능)
        inventoryRepo.addOrUpdateStock(domain::Inventory("13", 5));
        inventoryRepo.addOrUpdateStock(domain::Inventory("17", 6));
        std::cout << "  T2: 사이다(12), 탄산수(10), 아이스티(5), 이온음료(6) 설정됨." << std::endl;
    }
    // 다른 자판기들에 대해서도 필요에 따라 다양한 재고 설정
    else { 
    }
}