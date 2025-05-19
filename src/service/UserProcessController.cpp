#include "service/UserProcessController.hpp"
#include "service/MessageService.hpp" 
#include "persistence/inventoryRepository.h"
#include <iostream>
#include <string>
#include <vector>

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
        cerr << "[UC4] 결제 요청 시작" << endl;

        paymentReceiver.simulatePrepayment(
            [this, order](bool success) mutable {
                try {
                    if (success) {
                        // UC5: 결제 승인
                        orderService.approve(order.vmId(), true);
                        ui.displayMessage("결제 성공: " + order.drink().getName());
                    } else {
                        // UC6: 결제 거절
                        orderService.approve(order.vmId(), false);
                        ui.displayMessage("결제 거절: " + order.drink().getName());
                        handleMenu();  // 다시 UC1으로
                    }
                } catch (const exception& e) {
                    string err = e.what();
                    errorService.logError(err);
                    ui.show_error_message(err);
                    ui.display_Error(err);
                }
            },
            3  // delay_seconds: 3초
        );

    } catch (const exception& e) {
        string err = e.what();
        errorService.logError(err);
        ui.show_error_message(err);
        ui.display_Error(err);
    }
}

void UserProcessController::handlePrepayCode() {
    try {
        // UC13: 사용자에게 인증코드 입력 받기
        string code = ui.promptPrepayCode();

        // UC14: 인증코드 유효성 검사
        bool isValid = prepaymentService.isValid(code);

        if (isValid) {
            // UC7: 음료 배출 요청
            Drink drink = inventoryService.ReqReduceDrink(code);
            ui.dispense(drink);  // "콜라가 나왔습니다" 등 출력

            cerr << "[UC14] 인증코드 유효성 검증 완료" << endl;
            cerr << "[UC7] 음료 배출: " << drink.getName() << endl;
            cerr << "[UC14] changeStatusCode(" << code << ")" << endl;

            // UC14: 인증코드 상태 변경
            prepaymentService.changeStatusCode(code);
        } else {
            // UC13: 인증 실패 → 에러 처리
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

void UserProcessController::handleDrinkSelection() {
    string drinkName;
    cin >> drinkName;

    try {
        vector<inventory> drinks = inventoryRepository::getAllDrinks();

    bool valid = inventoryService.getSaleValid(drinkName);

    std::cout << "유효성 검사 결과: " << (valid ? "유효함 -> UC4 payment" : "유효하지 않음 -> uc8 BroadCast") << std::endl;

/*
try {
        vector<domain::inventory> drinks = inventoryRepository::getAllDrinks();


        for (const auto& drink : drinks) {
            if (drink.getDrink().getName() == drinkName) {
                found = true;

                // UC3: 현재 자판기 재고 확인
                bool valid = inventoryService.getSaleValid(drink.getDrink().getCode());

                if (valid) {
                    // UC4: 사용자 결제 요청
                    Order order("T1", drink.getDrink());  // 자판기 ID는 예시로 "T1"

                    if (ui.promptPrepayConsent()) {
                        string cardInfo = ui.promptCardInfo();

                        // UC4 → UC5 or UC6으로 이어짐
                        handlePayment(cardInfo, order);
                    } else {
                        handleMenu();  // 결제 안하고 다시 메뉴로
                    }
                } else {
                    // UC8: 재고 없음 → BroadCast
                    messageService.broadcastStock(drink.getDrink());

                    string err = "해당 자판기에 재고가 없습니다.";
                    errorService.logError(err);
                    ui.show_error_message(err);
                    ui.display_Error(err);
                }

                return;
            }
        }

        if (!found) {
            string err = "선택한 음료를 찾을 수 없습니다.";
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

*/
    
}
