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
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <unordered_map>
#include <algorithm> // std::find_if
#include <memory>    // std::make_shared

// --- 현재 자판기 기본 정보 (명령줄 인자로 덮어쓸 수 있음) ---
// 이 값들은 프로그램이 인자 없이 실행될 때 사용될 기본값입니다.
std::string THIS_VM_ID = "T1";
int THIS_VM_X = 10;
int THIS_VM_Y = 10;
unsigned short THIS_VM_PORT = 12345;

// --- 시스템에 존재하는 모든 자판기 정보 정의 ---
// OvmAddressRepository와 MessageSender 초기화에 사용됩니다.
// 각 자판기는 고유한 ID와 포트 번호를 가져야 합니다.
const std::vector<domain::VendingMachine> ALL_VENDING_MACHINES_IN_SYSTEM = {
    domain::VendingMachine("T1", 10, 10, "12345"),
    domain::VendingMachine("T2", 20, 20, "12346"),
    domain::VendingMachine("T3", 10, 30, "12347"),
    domain::VendingMachine("T4", 40, 10, "12348"),
    domain::VendingMachine("T5", 50, 50, "12349"),
    domain::VendingMachine("T6", 25, 40, "12350"),
    domain::VendingMachine("T7", 35, 25, "12351"),
    domain::VendingMachine("T8", 45, 35, "12352")
    // 실제 테스트 시, 이 목록에 정의된 모든 자판기가 실행될 필요는 없습니다.
    // MessageSender는 여기에 정의된 모든 "다른" 자판기에게 연결을 시도합니다.
};

/**
 * @brief 주어진 자판기 ID에 해당하는 VendingMachine 객체를 전체 자판기 목록에서 찾습니다.
 * @param vmId 찾을 자판기의 ID.
 * @param vmList 검색할 VendingMachine 객체들의 벡터.
 * @return 찾은 경우 VendingMachine 객체, 없으면 기본 생성된 VendingMachine.
 */
domain::VendingMachine findVmDefinitionInList(const std::string& vmId, const std::vector<domain::VendingMachine>& vmList) {
    auto it = std::find_if(vmList.begin(), vmList.end(),
                           [&vmId](const domain::VendingMachine& vm){ return vm.getId() == vmId; });
    if (it != vmList.end()) {
        return *it;
    }
    return domain::VendingMachine(); // 못 찾으면 빈 객체
}

int main(int argc, char* argv[]) {
    // 프로그램 실행 인자를 통해 현재 자판기의 ID, X, Y, Port를 설정합니다.
    // 인자가 없으면 위에 정의된 기본값을 사용합니다.
    if (argc == 5) { // 프로그램 이름 + 4개 인자
        THIS_VM_ID = argv[1];
        try {
            THIS_VM_X = std::stoi(argv[2]);
            THIS_VM_Y = std::stoi(argv[3]);
            THIS_VM_PORT = static_cast<unsigned short>(std::stoi(argv[4]));
        } catch (const std::exception& e) {
            std::cerr << "오류: 실행 인자 (X, Y, Port) 변환 실패. "
                      << "ID(" << THIS_VM_ID << ")에 대한 기본 설정을 시도합니다. 오류: " << e.what() << std::endl;
            domain::VendingMachine vmDef = findVmDefinitionInList(THIS_VM_ID, ALL_VENDING_MACHINES_IN_SYSTEM);
            if (!vmDef.getId().empty()) {
                THIS_VM_X = vmDef.getLocation().first;
                THIS_VM_Y = vmDef.getLocation().second;
                try {
                    THIS_VM_PORT = static_cast<unsigned short>(std::stoi(vmDef.getPort()));
                } catch (const std::exception& port_e) {
                     std::cerr << "오류: 기본 설정된 포트 번호(" << vmDef.getPort() << ") 변환 실패. 프로그램을 종료합니다. " << port_e.what() << std::endl;
                     return 1;
                }
            } else {
                 std::cerr << "오류: 자판기 ID(" << THIS_VM_ID << ")에 대한 기본 설정 정보를 찾을 수 없습니다. 프로그램을 종료합니다." << std::endl;
                 return 1;
            }
        }
    } else if (argc != 1) { // 인자가 아예 없거나, 개수가 맞지 않는 경우
        std::cerr << "사용법: " << argv[0] << " [자판기ID X좌표 Y좌표 포트번호]" << std::endl;
        std::cerr << "예시: " << argv[0] << " T1 10 10 12345" << std::endl;
        std::cerr << "인자 없이 실행 시 기본값 (ID: " << THIS_VM_ID
                  << ", X: " << THIS_VM_X << ", Y: " << THIS_VM_Y << ", Port: " << THIS_VM_PORT
                  << ")으로 실행됩니다." << std::endl;
        // 인자가 없으면 위에 정의된 전역 기본값을 사용합니다.
    }

    std::cout << "--- 자판기 " << THIS_VM_ID << " (Port: " << THIS_VM_PORT
              << ", X: " << THIS_VM_X << ", Y: " << THIS_VM_Y << ") 시스템 시작 ---" << std::endl;

    try {
        boost::asio::io_context io_context;
        presentation::UserInterface ui;

        // --- Repository 객체 생성 ---
        persistence::DrinkRepository drinkRepository;
        persistence::InventoryRepository inventoryRepository;
        persistence::OrderRepository orderRepository;
        persistence::OvmAddressRepository ovmAddressRepository;
        persistence::PrepayCodeRepository prepayCodeRepository;

        // --- 현재 자판기(THIS_VM_ID)의 초기 재고 설정 (테스트 시나리오에 맞게 수정) ---
        ui.displayMessage("[초기 설정] 현재 자판기(" + THIS_VM_ID + ")의 재고를 설정합니다...");
        if (THIS_VM_ID == "T1") {
            inventoryRepository.addOrUpdateStock(domain::Inventory("01", 5));  // 콜라 5개
            inventoryRepository.addOrUpdateStock(domain::Inventory("02", 0));  // 사이다 재고 없음
            inventoryRepository.addOrUpdateStock(domain::Inventory("03", 3));
        } else if (THIS_VM_ID == "T2") {
            inventoryRepository.addOrUpdateStock(domain::Inventory("01", 0));  // 콜라 재고 없음
            inventoryRepository.addOrUpdateStock(domain::Inventory("02", 10)); // 사이다 10개
            inventoryRepository.addOrUpdateStock(domain::Inventory("08", 7));
        } else if (THIS_VM_ID == "T3") {
            inventoryRepository.addOrUpdateStock(domain::Inventory("01", 2));
            inventoryRepository.addOrUpdateStock(domain::Inventory("02", 2));
            inventoryRepository.addOrUpdateStock(domain::Inventory("03", 2));
        }
        // ... 다른 자판기 ID에 대한 초기 재고 설정 ...
        ui.displayMessage("[초기 설정] 재고 설정 완료.");


        // --- 테스트용 선결제 코드 미리 저장 (UC13, UC14 테스트용) ---
        // 이 코드는 THIS_VM_ID 자판기에서 사용될 수 있는 코드입니다.
        ui.displayMessage("[초기 설정] 테스트용 선결제 코드를 저장합니다...");
        std::string testAuthCodeValid = THIS_VM_ID + "AVAL"; // 각 자판기마다 다른 유효한 테스트 코드 생성
        testAuthCodeValid.resize(5, 'X'); // 5자리로 맞춤
        std::string testAuthDrinkCode = "01"; // 예: 콜라

        // 현재 자판기에 해당 음료 재고가 있어야 선결제 코드가 의미 있음
        auto checkInv = inventoryRepository.getInventoryByDrinkCode(testAuthDrinkCode);
        if (!checkInv.getDrinkCode().empty() && checkInv.getQty() > 0) {
            auto orderForTestValid = std::make_shared<domain::Order>("VM_FROM_OTHER", testAuthDrinkCode, 1, testAuthCodeValid, "APPROVED");
            orderRepository.save(*orderForTestValid);
            prepayCodeRepository.save(domain::PrePaymentCode(testAuthCodeValid, domain::CodeStatus::ACTIVE, orderForTestValid));
            // 이 선결제로 인해 재고가 1개 줄어야 함 (테스트 시나리오를 위해)
            inventoryRepository.decreaseStockByOne(testAuthDrinkCode);
            ui.displayMessage("  - 사용 가능한 테스트 인증코드: " + testAuthCodeValid + " (음료: " + testAuthDrinkCode + ")");
        } else {
            ui.displayMessage("  - 알림: 테스트용 선결제 코드(" + testAuthCodeValid + ", 음료 " + testAuthDrinkCode + ")는 현재 재고가 없어 저장하지 않습니다.");
        }

        std::string testAuthCodeUsed = THIS_VM_ID + "USED";
        testAuthCodeUsed.resize(5, 'Y');
        auto orderForTestUsed = std::make_shared<domain::Order>("VM_FROM_OTHER2", testAuthDrinkCode, 1, testAuthCodeUsed, "APPROVED");
        orderRepository.save(*orderForTestUsed);
        domain::PrePaymentCode usedCode(testAuthCodeUsed, domain::CodeStatus::ACTIVE, orderForTestUsed);
        usedCode.markAsUsed(); // 이미 사용된 코드로 만듦
        prepayCodeRepository.save(usedCode);
        ui.displayMessage("  - 이미 사용된 테스트 인증코드: " + testAuthCodeUsed + " (음료: " + testAuthDrinkCode + ")");
        ui.displayMessage("[초기 설정] 선결제 코드 저장 완료.");


        // --- 다른 자판기 정보 OvmAddressRepository에 등록 및 MessageSender 설정 ---
        std::vector<std::string> other_vm_endpoints_for_sender;
        std::unordered_map<std::string, std::string> id_to_endpoint_map_for_sender;

        ui.displayMessage("[초기 설정] 다른 자판기 정보를 설정합니다 (통신 테스트용)...");
        for (const auto& vm_def : ALL_VENDING_MACHINES_IN_SYSTEM) {
            if (vm_def.getId() != THIS_VM_ID) { // 자기 자신은 제외
                ovmAddressRepository.save(vm_def);
                // 로컬 테스트를 위해 호스트는 "127.0.0.1"로 가정
                std::string endpoint_str = "127.0.0.1:" + vm_def.getPort();
                other_vm_endpoints_for_sender.push_back(endpoint_str);
                id_to_endpoint_map_for_sender[vm_def.getId()] = endpoint_str;
                ui.displayMessage("  - " + vm_def.getId() + " (" + endpoint_str + ") 정보 추가됨.");
            }
        }
        if (other_vm_endpoints_for_sender.empty()) {
            ui.displayMessage("  - 다른 자판기 정보가 설정되지 않았습니다. (브로드캐스트 시도 안 함)");
        }
        ui.displayMessage("[초기 설정] 다른 자판기 정보 설정 완료.");


        // --- Network Layer 객체 생성 ---
        network::MessageSender messageSender(io_context, other_vm_endpoints_for_sender, id_to_endpoint_map_for_sender);
        network::MessageReceiver messageReceiver(io_context, THIS_VM_PORT);

        // --- Service Layer 객체 생성 ---
        service::ErrorService errorService;
        service::InventoryService inventoryService(inventoryRepository, drinkRepository, errorService);
        service::DistanceService distanceService;
        service::PrepaymentService prepaymentService(prepayCodeRepository, orderRepository, errorService);
        service::MessageService messageService(messageSender, messageReceiver, errorService, THIS_VM_ID, THIS_VM_X, THIS_VM_Y);
        service::OrderService orderService(
            orderRepository, drinkRepository, inventoryService, prepaymentService, errorService
        );
        
        // UserProcessController 생성자에서 total_other_vms_ 값을 정확히 설정하도록 수정 필요
        // 예: controller(..., other_vm_endpoints_for_sender.size());
        // 현재 UserProcessController는 total_other_vms_를 내부적으로 7로 고정하고 있음.
        // 테스트를 위해 UserProcessController 생성자 또는 멤버를 수정하여
        // other_vm_endpoints_for_sender.size() 값을 사용하도록 하는 것이 좋음.
        // 여기서는 UserProcessController의 total_other_vms_가 7로 유지된다고 가정하고,
        // ALL_VENDING_MACHINES_IN_SYSTEM 목록이 8개일 때 정상 동작 기대.
        // 만약 ALL_VENDING_MACHINES_IN_SYSTEM 목록이 더 적다면, total_other_vms_ 값도 줄여야 함.
        service::UserProcessController controller(
            ui, inventoryService, orderService, prepaymentService,
            messageService, distanceService, errorService,
            io_context, THIS_VM_ID, THIS_VM_X, THIS_VM_Y
            // , other_vm_endpoints_for_sender.size() // UserProcessController 생성자 수정 시 이렇게 전달
        );

        ui.displayMessage("\n시스템 준비 완료. 사용자 입력을 기다립니다...");
        controller.run(); // 자판기 시스템 실행

    } catch (const std::exception& e) {
        std::cerr << "시스템 전체 오류 발생 (" << THIS_VM_ID << "): " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "알 수 없는 시스템 전체 오류 발생 (" << THIS_VM_ID << ")." << std::endl;
        return 1;
    }

    std::cout << "--- 자판기 " << THIS_VM_ID << " 시스템 종료 ---" << std::endl;
    return 0;
}
