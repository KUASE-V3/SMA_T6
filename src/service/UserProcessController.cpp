#include "service/UserProcessController.hpp"
#include "persistence/inventoryRepository.h"
#include <iostream>
#include "network/PaymentCallbackReceiver.hpp"

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

void UserProcessController::handlePayment(/*const bool& isPrepay*/) {
    try {
        network::PaymentCallbackReceiver receiver;
        //bool isPre = isPrepay;

        receiver.simulatePrepayment([/*isPrepay*/this](bool success) {
             // 지금 문서상으로는 결제의 종류를 가져올 수는 있지만 어떤 종류의 결제인지는 알 수 없음 리팩토링 필요 
             // 분기를 바깥에서 처리하고 이 함수에서는 결제 결과만 처리해야함 
                if (success) {
                    std::cout << "결제가 승인되었습니다. -> UC5" << std::endl;
                    if(true){// 결제 성공 후 처리
                        //선결제인경우
                        std::cout << "재고 확보 요청을 전송합니다 -> UC16" << std::endl;
                        OrderService orderService;
                        std::cout << "인증코드를 발급합니다. -> UC12" << std::endl;
                        prepayFlow_UC12();
                    }else{
                        std::cout << "음료를 배출합니다 -> UC7" << std::endl;
                        //결제 후 음료 배출
                    }
                } else {
                    std::cout << "결제가 거절되었습니다. -> UC6" << std::endl;
                    // 결제 실패 후 처리
                }
            }
        );

        /*
        string response = "Approve";  // 결제 ?��?�� �??��

        if (response == "Approve") {
            orderService.approve(order.vmId(), true); // UC5
            ui.displayMessage("결제 ?���?: " + order.drink().getName());
        } else {
            orderService.approve(order.vmId(), false); // UC6
            ui.displayMessage("결제 거절: " + order.drink().getName());
            handleMenu();
        }
        */ 
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
    cout << "음료수를 선택하세요: ";
    cin >> drinkName;

    bool valid = inventoryService.getSaleValid(drinkName);

    if (valid) {    //UC3
        ui.promptCardInfo();         //카드 정보를 cardInfo 변수에 저장
        // 여기서 cardInfo를 사용한 추가 처리 가능

    } else {    //UC 8 브로드캐스트 조회
        std::cout << "유효하지 않음. -> UC8 " << std::endl;
    }

/*
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

*/
    
}

void nofifyError(const std::string& error) {
    
    // Handle error notification
    UserInterface ui;
    if (error != "error") {
        ui.show_error_message(error); //에러 메시지가 일반적인 error가 아닐 때 사용자에게 표사
    } 
    ui.displayMainMenu(); //다시 진입점으로 이동

}

void nearestVM(const network::Message& msg) {

    // 사용자가 위치한 곳의 자판기에서 가장 가깝고 구매하고자 하는 음료의 재고가있는 자판기의 위치를 안내한다
    UserInterface ui;
    std::string vmId = msg.src_id;
    std::string x_coord = msg.msg_content.at("coor_x");
    std::string y_coord = msg.msg_content.at("coor_y");
    
    ui.display_SomeText("가장 가까운 자판기는 " +  vmId + "입니다.\n" + " 좌표는 " + x_coord + ", " + y_coord + "입니다.\n");
}

void showPrepaymentCode(const std::string& text) {//자판기 위치 호출 시스템 
    UserInterface ui;
    ui.display_SomeText("귀하의 결제코드는 " + text + "입니다.");
}

void prepayFlow_UC12(){

}