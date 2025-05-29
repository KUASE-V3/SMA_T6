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
#include <set> // 특정 VM만 활성화하기 위해 사용

// --- 전체 시스템에 정의된 자판기 정보 ---
// 이 정보는 모든 자판기 인스턴스가 공유해야 하는 '설정'과 같습니다.
const std::vector<domain::VendingMachine> ALL_VENDING_MACHINES_IN_SYSTEM = {
    domain::VendingMachine("T1", 10, 10, "12345"), domain::VendingMachine("T2", 20, 20, "12346"),
    domain::VendingMachine("T3", 10, 30, "12347"), domain::VendingMachine("T4", 40, 10, "12348"),
    domain::VendingMachine("T5", 50, 50, "12349"), domain::VendingMachine("T6", 25, 40, "12350"),
    domain::VendingMachine("T7", 35, 25, "12351"), domain::VendingMachine("T8", 45, 35, "12352")
};

// --- 현재 실행될 자판기의 기본 정보 (명령행 인자로 덮어쓸 수 있음) ---
std::string THIS_VM_ID = "T5"; // 기본 실행 ID
int THIS_VM_X = 10;
int THIS_VM_Y = 10;
unsigned short THIS_VM_PORT = 12345;

// --- 테스트를 위해 통신 대상으로 삼을 다른 자판기 ID 목록 (비어있으면 모두 대상) ---
// 예: std::set<std::string> TARGET_OTHER_VMS_FOR_TEST = {"T2", "T3"}; // T2, T3와만 통신 시도
std::set<std::string> TARGET_OTHER_VMS_FOR_TEST = {}; // 비어있으면 자신을 제외한 모든 VM과 통신 시도

// 함수 선언
domain::VendingMachine findVmDefinitionInList(const std::string& vmId, const std::vector<domain::VendingMachine>& vmList);
void setupInitialInventory(const std::string& currentVmId, persistence::InventoryRepository& inventoryRepo);

int main(int argc, char* argv[]) {
    // 1. 명령행 인자 파싱 (자판기 ID, X, Y, Port 설정)
    if (argc >= 2 && std::string(argv[1]) == "--help") {
        std::cout << "사용법: " << argv[0] << " [자판기ID X Y Port] [--targets VM_ID1,VM_ID2,...]" << std::endl;
        std::cout << "  [자판기ID X Y Port]: 현재 실행할 자판기의 정보를 지정합니다." << std::endl;
        std::cout << "  --targets VM_ID1,VM_ID2,...: 통신 대상으로 삼을 다른 자판기 ID 목록을 콤마로 구분하여 지정합니다. (선택 사항)" << std::endl;
        std::cout << "인자 없이 실행 시 기본값 (" << THIS_VM_ID << ", X:" << THIS_VM_X << ", Y:" << THIS_VM_Y << ", Port:" << THIS_VM_PORT << ")으로 실행되며, 모든 다른 자판기와 통신을 시도합니다." << std::endl;
        return 0;
    }

    if (argc >= 5) { // ID X Y Port가 주어진 경우
        THIS_VM_ID = argv[1];
        try {
            THIS_VM_X = std::stoi(argv[2]);
            THIS_VM_Y = std::stoi(argv[3]);
            THIS_VM_PORT = static_cast<unsigned short>(std::stoi(argv[4]));
        } catch (const std::exception& e) {
            std::cerr << "오류: 제공된 실행 인자 (X, Y, Port) 변환 실패. ID(" << THIS_VM_ID << ")에 대한 기본 설정 시도. " << e.what() << std::endl;
            domain::VendingMachine vmDef = findVmDefinitionInList(THIS_VM_ID, ALL_VENDING_MACHINES_IN_SYSTEM);
            if (!vmDef.getId().empty()) {
                THIS_VM_X = vmDef.getLocation().first; THIS_VM_Y = vmDef.getLocation().second;
                try { THIS_VM_PORT = static_cast<unsigned short>(std::stoi(vmDef.getPort())); }
                catch (const std::exception& pe) { std::cerr << "오류: ID(" << THIS_VM_ID << ")의 기본 포트(" << vmDef.getPort() << ") 변환 실패. 종료. " << pe.what() << std::endl; return 1; }
            } else { std::cerr << "오류: ID(" << THIS_VM_ID << ")에 대한 정의를 찾을 수 없음. 종료." << std::endl; return 1; }
        }
        // 추가 인자 (통신 대상 지정) 파싱
        if (argc >= 7 && std::string(argv[5]) == "--targets") {
            std::string targetsStr = argv[6];
            size_t start = 0;
            size_t end = targetsStr.find(',');
            while (end != std::string::npos) {
                TARGET_OTHER_VMS_FOR_TEST.insert(targetsStr.substr(start, end - start));
                start = end + 1;
                end = targetsStr.find(',', start);
            }
            TARGET_OTHER_VMS_FOR_TEST.insert(targetsStr.substr(start, end)); // 마지막 ID 추가
            std::cout << "정보: 통신 대상 자판기가 ";
            for(const auto& id : TARGET_OTHER_VMS_FOR_TEST) std::cout << id << " ";
            std::cout << "로 제한됩니다." << std::endl;
        }
    } else if (argc != 1) { // 그 외 잘못된 인자 수
        std::cerr << "경고: 인자 수가 맞지 않습니다. 기본값(" << THIS_VM_ID << ")으로 실행합니다. 도움말은 --help 옵션을 사용하세요." << std::endl;
        // 기본값(THIS_VM_ID, X, Y, PORT) 사용 (main 함수 시작 전 설정된 값)
        domain::VendingMachine vmDef = findVmDefinitionInList(THIS_VM_ID, ALL_VENDING_MACHINES_IN_SYSTEM);
        if (!vmDef.getId().empty()) {
            THIS_VM_X = vmDef.getLocation().first; THIS_VM_Y = vmDef.getLocation().second;
            try { THIS_VM_PORT = static_cast<unsigned short>(std::stoi(vmDef.getPort())); }
            catch (const std::exception& e) { std::cerr << "오류: 기본 ID(" << THIS_VM_ID << ")의 포트 변환 실패. 종료. " << e.what() << std::endl; return 1;}
        } else {
             std::cerr << "오류: 기본 ID(" << THIS_VM_ID << ") 정의를 찾을 수 없음. 종료." << std::endl; return 1;
        }
    } else { // 인자 없이 실행 시 (argc == 1)
         domain::VendingMachine vmDef = findVmDefinitionInList(THIS_VM_ID, ALL_VENDING_MACHINES_IN_SYSTEM);
         if (!vmDef.getId().empty()) {
            THIS_VM_X = vmDef.getLocation().first; THIS_VM_Y = vmDef.getLocation().second;
            try { THIS_VM_PORT = static_cast<unsigned short>(std::stoi(vmDef.getPort())); }
            catch (const std::exception& e) { std::cerr << "오류: 기본 ID(" << THIS_VM_ID << ")의 포트 변환 실패. 종료. " << e.what() << std::endl; return 1;}
        } else {
             std::cerr << "오류: 기본 ID(" << THIS_VM_ID << ") 정의를 찾을 수 없음. 종료." << std::endl; return 1;
        }
        std::cout << "정보: 인자 없이 실행되어 기본값(" << THIS_VM_ID << ")으로 시작합니다." << std::endl;
    }

    std::cout << "--- 자판기 " << THIS_VM_ID << " (Port: " << THIS_VM_PORT << ", X: " << THIS_VM_X << ", Y: " << THIS_VM_Y << ") 시스템 시작 ---" << std::endl;

    try {
        // 2. 시스템 구성 요소 초기화
        boost::asio::io_context io_context;
        auto work_guard = boost::asio::make_work_guard(io_context); // io_context.run()이 즉시 종료되지 않도록 함

        presentation::UserInterface ui;
        persistence::DrinkRepository drinkRepository; // 모든 음료(20종) 기본 정보 로드 가정
        persistence::InventoryRepository inventoryRepository;
        persistence::OrderRepository orderRepository;
        persistence::OvmAddressRepository ovmAddressRepository; // 다른 자판기 주소 정보
        persistence::PrepayCodeRepository prepayCodeRepository;

        // 3. 현재 자판기의 초기 재고 설정
        setupInitialInventory(THIS_VM_ID, inventoryRepository);
        std::cout << "정보: " << THIS_VM_ID << " 자판기의 초기 재고 설정 완료." << std::endl;

        // 4. 다른 자판기 정보 설정 (MessageSender 및 OvmAddressRepository용)
        std::vector<std::string> other_vm_endpoints_for_sender; // MessageSender의 브로드캐스트용
        std::unordered_map<std::string, std::string> id_to_endpoint_map_for_sender; // MessageSender의 유니캐스트용
        int actual_other_vm_count = 0;

        std::cout << "정보: 다른 자판기 정보 로드 중..." << std::endl;
        for (const auto& vm_def : ALL_VENDING_MACHINES_IN_SYSTEM) {
            if (vm_def.getId() == THIS_VM_ID) {
                continue; // 자기 자신은 제외
            }

            // --targets 옵션으로 지정된 경우, 해당 VM만 통신 대상으로 추가
            if (!TARGET_OTHER_VMS_FOR_TEST.empty() && TARGET_OTHER_VMS_FOR_TEST.find(vm_def.getId()) == TARGET_OTHER_VMS_FOR_TEST.end()) {
                std::cout << "  - " << vm_def.getId() << " (" << vm_def.getPort() << "): 통신 대상에서 제외됨 (지정 목록에 없음)" << std::endl;
                continue;
            }

            ovmAddressRepository.save(vm_def); // OvmAddressRepository에 다른 자판기 정보 저장
            std::string endpoint_str = "127.0.0.1:" + vm_def.getPort(); // 로컬 테스트 환경 가정
            
            other_vm_endpoints_for_sender.push_back(endpoint_str);
            id_to_endpoint_map_for_sender[vm_def.getId()] = endpoint_str;
            actual_other_vm_count++;
            std::cout << "  + " << vm_def.getId() << " (Port: " << vm_def.getPort() << ", Endpoint: " << endpoint_str << ") 통신 대상으로 추가됨." << std::endl;
        }
        if (actual_other_vm_count == 0 && TARGET_OTHER_VMS_FOR_TEST.empty() && ALL_VENDING_MACHINES_IN_SYSTEM.size() > 1) {
             std::cout << "경고: 다른 자판기가 시스템에 정의되어 있으나, 통신 대상으로 설정된 다른 자판기가 없습니다." << std::endl;
        } else if (actual_other_vm_count == 0 && !TARGET_OTHER_VMS_FOR_TEST.empty()){
             std::cout << "경고: --targets 옵션으로 지정된 다른 자판기가 없거나 시스템 정의와 일치하지 않습니다." << std::endl;
        }


        // 5. 서비스 및 컨트롤러 계층 초기화
        network::MessageSender messageSender(io_context, other_vm_endpoints_for_sender, id_to_endpoint_map_for_sender);
        network::MessageReceiver messageReceiver(io_context, THIS_VM_PORT); // 현재 자판기가 수신할 포트

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
            actual_other_vm_count // UserProcessController가 알아야 할 '다른 자판기 총 수'
        );

        // 6. io_context 실행 스레드 시작
        std::thread io_thread([&io_context, vm_id = THIS_VM_ID](){
            try {
                std::cout << "정보: " << vm_id << "의 io_context 스레드 시작." << std::endl;
                io_context.run(); // 메시지 수신 및 비동기 작업 처리
                std::cout << "정보: " << vm_id << "의 io_context 스레드 정상 종료." << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "오류: " << vm_id << "의 io_context 스레드에서 예외 발생: " << e.what() << std::endl;
            }
        });

        // 7. UserProcessController 메인 루프 실행
        controller.run();

        // 8. 시스템 종료 처리
        std::cout << "정보: " << THIS_VM_ID << " 자판기 메인 루프 종료. 시스템 종료 절차 시작..." << std::endl;
        io_context.stop();    // io_context의 모든 작업 중단 요청 (진행 중인 비동기 작업 완료 후 run() 반환 유도)
        work_guard.reset();   // io_context.run()이 즉시 종료되지 않도록 했던 work_guard 해제
        if (io_thread.joinable()) {
            std::cout << "정보: " << THIS_VM_ID << " io_context 스레드 조인 대기..." << std::endl;
            io_thread.join(); // io_context 스레드 완전히 종료될 때까지 대기
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

// --- Helper Functions ---

domain::VendingMachine findVmDefinitionInList(const std::string& vmId, const std::vector<domain::VendingMachine>& vmList) {
    auto it = std::find_if(vmList.begin(), vmList.end(),
                           [&vmId](const domain::VendingMachine& vm){ return vm.getId() == vmId; });
    if (it != vmList.end()) return *it;
    return domain::VendingMachine(); // 빈 객체 반환 (ID가 비어있음)
}

void setupInitialInventory(const std::string& currentVmId, persistence::InventoryRepository& inventoryRepo) {
    // 여기에 각 자판기 ID별로 초기 재고를 설정합니다.
    // PFR 1.2: 하나의 자판기는 7종류의 음료를 취급한다고 가정. (실제 취급 음료 및 개수는 자유롭게 설정)
    // PFR R.98: 각 음료의 최대 재고는 99개로 한정.
    std::cout << "정보: " << currentVmId << " 자판기의 재고를 설정합니다." << std::endl;
    if (currentVmId == "T1") {
        inventoryRepo.addOrUpdateStock(domain::Inventory("01", 5));  // 콜라
        inventoryRepo.addOrUpdateStock(domain::Inventory("02", 0));  // 사이다 (재고 없음)
        inventoryRepo.addOrUpdateStock(domain::Inventory("03", 10)); // 녹차
        inventoryRepo.addOrUpdateStock(domain::Inventory("04", 2));  // 홍차
        inventoryRepo.addOrUpdateStock(domain::Inventory("05", 7));  // 밀크티
        inventoryRepo.addOrUpdateStock(domain::Inventory("06", 1));  // 탄산수
        inventoryRepo.addOrUpdateStock(domain::Inventory("07", 4));  // 보리차
        std::cout << "  T1: 콜라(5), 사이다(0), 녹차(10), 홍차(2), 밀크티(7), 탄산수(1), 보리차(4) 설정됨." << std::endl;
    } else if (currentVmId == "T2") {
        inventoryRepo.addOrUpdateStock(domain::Inventory("01", 0));  // 콜라 (재고 없음)
        inventoryRepo.addOrUpdateStock(domain::Inventory("02", 12)); // 사이다
        inventoryRepo.addOrUpdateStock(domain::Inventory("08", 8));  // 캔커피
        inventoryRepo.addOrUpdateStock(domain::Inventory("09", 15)); // 물
        inventoryRepo.addOrUpdateStock(domain::Inventory("10", 3));  // 에너지드링크
        inventoryRepo.addOrUpdateStock(domain::Inventory("11", 6));  // 유자차
        inventoryRepo.addOrUpdateStock(domain::Inventory("12", 9));  // 식혜
        std::cout << "  T2: 콜라(0), 사이다(12), 캔커피(8), 물(15), 에너지드링크(3), 유자차(6), 식혜(9) 설정됨." << std::endl;
    } else if (currentVmId == "T3") {
        inventoryRepo.addOrUpdateStock(domain::Inventory("01", 3));  // 콜라
        // T3는 콜라만 판매한다고 가정 (테스트 목적)
        inventoryRepo.addOrUpdateStock(domain::Inventory("13", 5));
        inventoryRepo.addOrUpdateStock(domain::Inventory("14", 5));
        inventoryRepo.addOrUpdateStock(domain::Inventory("15", 5));
        inventoryRepo.addOrUpdateStock(domain::Inventory("16", 5));
        inventoryRepo.addOrUpdateStock(domain::Inventory("17", 5));
        inventoryRepo.addOrUpdateStock(domain::Inventory("18", 5));
        std::cout << "  T3: 콜라(3) 및 기타 음료 설정됨." << std::endl;

    } else if (currentVmId == "T5") { // 실행 결과에서 사용된 T5
        inventoryRepo.addOrUpdateStock(domain::Inventory("01", 0));  // 콜라 (재고 없음)
        inventoryRepo.addOrUpdateStock(domain::Inventory("02", 0));  // 사이다 (재고 없음)
        inventoryRepo.addOrUpdateStock(domain::Inventory("03", 10)); // 녹차
        inventoryRepo.addOrUpdateStock(domain::Inventory("04", 10)); // 홍차
        inventoryRepo.addOrUpdateStock(domain::Inventory("05", 10)); // 밀크티
        inventoryRepo.addOrUpdateStock(domain::Inventory("06", 10)); // 탄산수
        inventoryRepo.addOrUpdateStock(domain::Inventory("07", 10)); // 보리차
        std::cout << "  T5: 콜라(0), 사이다(0), 녹차(10), 홍차(10), 밀크티(10), 탄산수(10), 보리차(10) 설정됨." << std::endl;
    }
    // 다른 자판기 (T4, T6, T7, T8)에 대한 재고 설정도 위와 같이 추가합니다.
    // 예시:
    // else if (currentVmId == "T4") { ... }
    else {
        // 기본적으로 모든 음료 1개씩 재고 보유 (테스트용)
        for (int i = 1; i <= 20; ++i) {
            std::string drinkCode = (i < 10) ? "0" + std::to_string(i) : std::to_string(i);
            inventoryRepo.addOrUpdateStock(domain::Inventory(drinkCode, 1));
        }
        std::cout << "  " << currentVmId << ": 정의되지 않은 ID로, 모든 음료 기본 재고 1개 설정됨 (테스트용)." << std::endl;
    }
}