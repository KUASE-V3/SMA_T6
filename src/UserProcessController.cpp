// UserProcessController.cpp
// ------------------------------
#include "UserProcessController.hpp"

UserProcessController::UserProcessController() : orderService(&order) {}


void UserProcessController::handleMenu() {
    try {
        std::vector<std::pair<std::string, int>> list = inventoryService.getList();
        ui.displayList(list);
    } catch (const std::exception& e) {
        std::string err = e.what();
        errorService.log_error(err);
        errorService.notify_error(err);
        ui.show_error_message(err);
        ui.display_Error(err);
    }
}

void UserProcessController::handlePayment(const std::string& cardInfo) {
    try {
        std::string response = paymentReceiver.callReceiver(cardInfo); // 응답: "Approve" or "Declined"

        if (response == "Approve") {
            orderService.approve("paymentID", true); // UC5
            ui.displayMessage("결제 성공");
        } else {
            orderService.approve("paymentID", false); // UC6
            ui.displayMessage("결제 거절");
            handleMenu(); // UC1: 목록 조회로 이동
        }
    } catch (const std::exception& e) {
        std::string err = e.what();
        errorService.log_error(err);
        errorService.notify_error(err);
        ui.show_error_message(err);
        ui.display_Error(err);
    }
}

void UserProcessController::handlePrepayCode() {
    try {
        std::string code = ui.promptPrepayCode();
        bool usable = prepayService.isValid(code);

        if (usable) {
            PrepayCode p;
            bool same = p.isSameCode(code);
            if (same) {
                p.use(code);
                p.changeStatusCode(code);
                std::cout << "[UC14] 인증코드 유효 확인 완료" << std::endl;

                std::string drinkCode = p.getDrinkCode(code);
                std::cout << "[UC7] 음료 코드 " << drinkCode << " 에 해당하는 음료가 배출되었습니다." << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::string err = e.what();
        errorService.log_error(err);
        errorService.notify_error(err);
        ui.show_error_message(err);
        ui.display_Error(err);
    }
}

void UserProcessController::handleDrinkSelection() {
    std::string drinkName;
    std::cout << "음료수를 입력하세요: ";
    std::cin >> drinkName;

    try {
        // 음료 이름 → 고유 코드로 매핑
        std::string drinkCode = inventoryService.getDrinkCodeByName(drinkName);

        bool valid = inventoryService.getSaleValid(drinkCode);

        if (valid) {
            // 재고 있음 → 선결제 여부 확인
            if (ui.promptPrepayConsent()) {
                std::string cardInfo = ui.promptCardInfo();
                handlePayment(cardInfo); // UC4 결제 요청
            } else {
                handleMenu(); // UC1: 목록 조회로 되돌아감
            }
            return;
        }

        // 재고 없음 처리 (UC8, UC9, UC11 등)
        // ... 생략 ...

    } catch (const std::exception& e) {
        std::string err = e.what();
        errorService.log_error(err);
        errorService.notify_error(err);
        ui.show_error_message(err);
        ui.display_Error(err);
    }
}

