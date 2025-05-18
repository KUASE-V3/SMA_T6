#include "service/UserProcessController.hpp"
#include "persistence/inventoryRepository.h"
#include <iostream>

using namespace std;
using namespace domain;
using namespace persistence;
using namespace service;

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

void UserProcessController::handlePayment(const string& cardInfo, Order& order) {
    try {
        string response = "Approve";  // 결제 ?��?�� �??��

        if (response == "Approve") {
            orderService.approve(order.vmId(), true); // UC5
            ui.displayMessage("결제 ?���?: " + order.drink().getName());
        } else {
            orderService.approve(order.vmId(), false); // UC6
            ui.displayMessage("결제 거절: " + order.drink().getName());
            handleMenu();
        }
    } catch (const exception& e) {
        string err = e.what();
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
            cout << "[UC14] ?��증코?�� ?��?��?�� �??�� ?���?" << endl;
            cout << "[UC7] ?���? 배출 ?���?" << endl;
            cout << "[UC14] ?��?�� 코드 �?�? ?���? ?�� changeStatusCode(" << code << ")" << endl;
        } else {
            string err = "?��?��?���? ?��??? ?��증코?��?��?��?��.";
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

void UserProcessController::handleDrinkSelection() {
    string drinkName;
    cout << "?��료수�? ?��?��?��?��?��: ";
    cin >> drinkName;

    try {
        vector<domain::inventory> drinks = inventoryRepository::getAllDrinks();

        bool found = false;

        for (const auto& drink : drinks) {
            if (drink.getDrink().getName() == drinkName) {
                found = true;

                bool valid = inventoryService.getSaleValid(drink.getDrink().getCode());
                if (valid) {
                    Order order("T1", drink.getDrink());  // ?��?���? ID?�� "T1" �??��

                    if (ui.promptPrepayConsent()) {
                        string cardInfo = ui.promptCardInfo();
                        handlePayment(cardInfo, order);  // ?��메인 객체 ?��?��
                    } else {
                        handleMenu();
                    }
                    return;
                } else {
                    string err = "?��고�?? �?족합?��?��.";
                    errorService.logError(err);
                    ui.show_error_message(err);
                    ui.display_Error(err);
                    return;
                }
            }
        }

        if (!found) {
            string err = "?��?�� ?��료�?? 찾을 ?�� ?��?��?��?��.";
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
