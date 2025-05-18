#include "service/UserProcessController.hpp"
#include "persistence/inventoryRepository.h"
#include <iostream>

using namespace std;
using namespace domain;
using namespace persistence;

UserProcessController::UserProcessController() : orderService() {}

void UserProcessController::handleMenu() {
    try {
        vector<pair<string, int>> list = inventoryService.CallInventorySer();
        ui.displayList(list);
    } catch (const exception& e) {
        string err = e.what();
        ErrorService::logError(err);
        ui.show_error_message(err);
        ui.display_Error(err);
    }
}

void UserProcessController::handlePayment(const string& cardInfo, Order& order) {
    try {
        string response = "Approve";  // 결제 응답 가정

        if (response == "Approve") {
            orderService.approve(order.vmId(), true); // UC5
            ui.displayMessage("결제 성공: " + order.drink().getName());
        } else {
            orderService.approve(order.vmId(), false); // UC6
            ui.displayMessage("결제 거절: " + order.drink().getName());
            handleMenu();
        }
    } catch (const exception& e) {
        string err = e.what();
        ErrorService::logError(err);
        ui.show_error_message(err);
        ui.display_Error(err);
    }
}

void UserProcessController::handlePrepayCode() {
    try {
        string code = ui.promptPrepayCode();

        bool isValid = prepaymentService.isValid(code);

        if (isValid) {
            cout << "[UC14] 인증코드 유효성 검사 성공" << endl;
            cout << "[UC7] 음료 배출 완료" << endl;
            cout << "[UC14] 상태 코드 변경 요청 → changeStatusCode(" << code << ")" << endl;
        } else {
            string err = "유효하지 않은 인증코드입니다.";
            ErrorService::logError(err);
            ui.show_error_message(err);
            ui.display_Error(err);
        }
    } catch (const exception& e) {
        string err = e.what();
        ErrorService::logError(err);
        ui.show_error_message(err);
        ui.display_Error(err);
    }
}

void UserProcessController::handleDrinkSelection() {
    string drinkName;
    cout << "음료수를 입력하세요: ";
    cin >> drinkName;

    try {
        vector<Drink> drinks = inventoryRepository::getAllDrinks();  // 도메인 객체 리스트
        bool found = false;

        for (const auto& drink : drinks) {
            if (drink.getName() == drinkName) {
                found = true;

                bool valid = inventoryService.getSaleValid(drink.getCode());
                if (valid) {
                    Order order("T1", drink);  // 자판기 ID는 "T1" 가정

                    if (ui.promptPrepayConsent()) {
                        string cardInfo = ui.promptCardInfo();
                        handlePayment(cardInfo, order);  // 도메인 객체 전달
                    } else {
                        handleMenu();
                    }
                    return;
                } else {
                    string err = "재고가 부족합니다.";
                    ErrorService::logError(err);
                    ui.show_error_message(err);
                    ui.display_Error(err);
                    return;
                }
            }
        }

        if (!found) {
            string err = "해당 음료를 찾을 수 없습니다.";
            ErrorService::logError(err);
            ui.show_error_message(err);
            ui.display_Error(err);
        }

    } catch (const exception& e) {
        string err = e.what();
        ErrorService::logError(err);
        ui.show_error_message(err);
        ui.display_Error(err);
    }
}
