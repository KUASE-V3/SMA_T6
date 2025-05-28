#include "presentation/UserInterface.hpp"
#include <iostream>
#include "service/UserProcessController.hpp"

void UserInterface::displayMainMenu() {
    std::cout << "===== 자판기 메뉴 =====\n";
    std::cout << "1. 음료 목록 조회\n";
    std::cout << "2. 음료 선택\n";
    std::cout << "3. 선결제 확인\n";
    std::cout << "4. 종료\n";
    std::cout << "5. 인증코드 입력\n";
    std::cout << "번호를 선택하세요: ";
}

void UserInterface::displayList(const std::vector<std::pair<std::string, int>>& list) {
    std::cout << "[음료 목록]\n";
    for (const auto& drink : list) {
        std::cout << "이름: " << drink.first << ", 가격: " << drink.second << "원\n";
    }
}

void UserInterface::show_error_message(const std::string& error) {
    std::cerr << "[에러 메시지] " << error << std::endl;
}

void UserInterface::display_Error(const std::string& error) {
    std::cerr << "[표시 오류] " << error << std::endl;
}

void UserInterface::displayMessage(const std::string& msg) {
    std::cout << msg << std::endl;
}


bool UserInterface::isValidCardNumber(const std::string& cardNumber) {
    if (cardNumber.length() != 16) return false;
    for (char c : cardNumber) {
        if (!isdigit(c)) return false;
    }
    return true;
}


void UserInterface::promptCardInfo() {
    
    int choice = 0;
    bool isPrepay = false;

    while (true) {
        std::cout << "결제 방식을 선택하세요:\n";
        std::cout << "1. 일반 결제\n";
        std::cout << "2. 선결제\n";
        std::cout << "선택: ";
        std::cin >> choice;

        if (choice == 1) {
            isPrepay = false;
            break;
        } else if (choice == 2) {
            isPrepay = true;
            break;
        } else {
            std::cout << "잘못된 입력입니다. 다시 선택해주세요.\n";
        }
    }

    // 카드 번호 입력 & 유효성 검사
    std::string cardNumber;
    while (true) {
        std::cout << "카드 번호를 입력하세요 (숫자 16자리): ";
        std::cin >> cardNumber;

        if (isValidCardNumber(cardNumber)) {
            break;
        } else {
            std::cout << "유효하지 않은 카드 번호입니다. 숫자 16자리를 다시 입력해주세요.\n";
        }
    }

}

std::string UserInterface::promptPrepayCode() {
    std::string code;
    std::cout << "인증코드를 입력하세요: ";

    std::cin >> code;
    return code;
}

void UserInterface::dispense(const std::string& drinkCode) {
    std::cout << "[음료 배출] 코드 " << drinkCode << "에 해당하는 음료가 배출되었습니다.\n";
}

bool UserInterface::promptPrepayConsent() {
    char consent;
    std::cout << "[선결제 동의] 선결제에 동의하시겠습니까? (y/n): ";
    std::cin >> consent;
    return (consent == 'y' || consent == 'Y');
}

void UserInterface::showText(const std::string& prepaymentCode) {
    std::cout << "선결제 코드: " << prepaymentCode << std::endl;
}

void UserInterface::display_SomeText(const std::string& text) {
    std::cout << text << std::endl;
}
