#include "service/UserProcessController.hpp"
#include "persistence/inventoryRepository.h"
#include <iostream>
#include <algorithm> // for std::find_if, std::isspace
#include "network/PaymentCallbackReceiver.hpp"
#include "service/MessageService.hpp"

using namespace std;
using namespace domain;
using namespace persistence;
using namespace service;

// 🔧 trim 함수 정의 (앞뒤 공백 제거)
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

UserProcessController::UserProcessController() : orderService() {}

void UserProcessController::handleMenu() {
    try {
        vector<pair<string, int>> list = inventoryService.CallInventorySer();
        ui.displayList(list);
    } catch (const exception& e) {
        string err = e.what();
        errorService.logError(err);
        ui.show_error_message(err);
        ui.display_Error(err);
    }
}

void UserProcessController::handlePayment(const bool& isPrepay) {
    try {
        network::PaymentCallbackReceiver receiver;

        receiver.simulatePrepayment([isPrepay, this](bool success) {
            std::string temp_drink_id = "001";  // TODO: 실제 선택한 음료 코드로 대체
            std::string temp_cert_code = "TEMP"; // 선결제 코드가 없는 경우에도 필요하므로 기본값
            domain::Order order = this->orderService.createOrder(temp_drink_id, temp_cert_code);

            if (success) {
                std::cout << "결제성공" << std::endl;

                // 결제 승인 처리
                this->orderService.approve("PAY1234", true); // 임시 paymentID

                if (isPrepay) {
                    std::cout << "재고 확보 요청을 전송합니다 -> UC16" << std::endl;
                    std::cout << "인증코드를 발급합니다. -> UC12" << std::endl;
                    std::string code = prepayFlow_UC12();
                    // TODO: msgService.sendPrePayReq(order); // 생략
                } else {
                    std::cout << "음료를 배출합니다" << std::endl;
                    // TODO: 재고 감소 로직 추가 예정
                }

            } else {
                std::cout << "결제거절" << std::endl;

                this->orderService.approve("PAY1234", false); // 상태: Declined

                std::cout << "\n메인 메뉴로 돌아갑니다.\n" << std::endl;
            }
        });

    } catch (const std::exception& e) {
        std::string err = e.what();
        errorService.logError(err);
        ui.show_error_message(err);
        ui.display_Error(err);
    }
}

void UserProcessController::handlePrepayCode() {
    try {
        string code = ui.promptPrepayCode();

        bool isValid = prepaymentService.isValid(code);

        if (isValid) {
            cout << "[UC14] 인증코드 유효함 확인됨" << endl;
            cout << "[UC7] 음료 배출 진행" << endl;
            cout << "[UC14] 인증코드 상태 변경 중: changeStatusCode(" << code << ")" << endl;
        } else {
            string err = "유효하지 않은 인증코드입니다.";
            errorService.logError(err);
            ui.show_error_message(err);
            ui.display_Error(err);
        }
    } catch (const exception& e) {
        string err = e.what();
        errorService.logError(err);
        ui.show_error_message(err);
        ui.display_Error(err);
    }
}

// ✅ 여기가 공백 있는 입력 처리 핵심!
void UserProcessController::handleDrinkSelection() {
    std::string drinkName;
    std::cout << "음료수를 선택하세요: ";
    std::getline(std::cin >> std::ws, drinkName);  // 줄 전체 입력
    drinkName = trim(drinkName);                  // 앞뒤 공백 제거

    bool valid = inventoryService.getSaleValid(drinkName);

    if (valid) {
        ui.promptCardInfo(); // UC3
    } else {
        std::cout << "유효하지 않음. -> UC8" << std::endl;
    }
}

void UserProcessController::nofityError(const std::string& error) {
    UserInterface ui;
    if (error != "error") {
        ui.show_error_message(error);
    }
    ui.displayMainMenu();
}

void UserProcessController::nearestVM(const network::Message& msg) {
    UserInterface ui;
    std::string vmId = msg.src_id;
    std::string x_coord = msg.msg_content.at("coor_x");
    std::string y_coord = msg.msg_content.at("coor_y");

    ui.display_SomeText("가장 가까운 자판기는 " + vmId + "입니다.\n" +
                        "좌표는 " + x_coord + ", " + y_coord + "입니다.\n");
}

void UserProcessController::showPrepaymentCode(const std::string& text) {
    UserInterface ui;
    ui.display_SomeText("귀하의 결제코드는 " + text + "입니다.");
}

std::string UserProcessController::prepayFlow_UC12() {
    std::string prepayCode = prepaymentService.isSueCode();
    ui.display_SomeText(prepayCode);
    handlePayment(true); // 선결제 처리
    return prepayCode;
}
