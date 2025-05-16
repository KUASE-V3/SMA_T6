// ----- UserProcessController.cpp -----
#include "UserProcessController.h"
#include "InventoryService.h"
#include "ErrorService.h"
#include "UserInterface.h"
#include "OrderService.h"
#include "MessageService.h"
#include <iostream>
#include <stdexcept>

UserProcessController::UserProcessController(
    InventoryService* inv, ErrorService* err, UserInterface* ui,
    OrderService* ord, MessageService* msg
) : invService(inv), errService(err), ui(ui), orderService(ord), msgService(msg) {}

void UserProcessController::handleMenu() {
    try {
        auto list = invService->callInventorySer();
        ui->displayList(list);
    } catch (const std::exception& e) {
        std::string errMsg = errService->logError(e.what());
        ui->displayError(errMsg);
    }
}

void UserProcessController::handleDrinkSelection() {
    try {
        int index = ui->askDrinkIndex();
        std::string drink = invService->getDrinkByIndex(index);
        ui->showSelectedDrink(drink);

        if (invService->getSaleValid(index)) {
            broadcastStock(drink);
        } else {
            std::string cardInfo = ui->promptCardInfo();
            handlePayment(cardInfo);
        }
    } catch (const std::exception& e) {
        std::string errMsg = errService->logError(e.what());
        ui->displayError(errMsg);
    }
}

void UserProcessController::handlePayment(const std::string& cardInfo) {
    try {
        // 예시로 승인 조건 단순화
        bool success = (cardInfo == "valid");
        orderService->approve("payment-id", success);
        ui->showMessage(success ? "결제 성공" : "결제 거절");

        if (!success) {
            handleMenu();  // UC1으로 돌아감
        }
    } catch (const std::exception& e) {
        std::string errMsg = errService->logError(e.what());
        ui->displayError(errMsg);
    }
}

void UserProcessController::broadcastStock(const std::string& drink) {
    try {
        msgService->send("broadcast", drink + " 재고 부족");
    } catch (const std::exception& e) {
        std::string errMsg = errService->logError(e.what());
        errService->logError(errMsg);
    }
}
