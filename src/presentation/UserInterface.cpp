#include "UserInterface.hpp"
#include <iostream>

void UserInterface::displayMainMenu() {
    std::cout << "===== 자판기 메뉴 =====\n";
    std::cout << "1. 음료 목록 조회\n2. 음료 선택\n3. 재고 확인\n4. 종료\n5. 인증코드 입력\n번호를 선택하세요: ";
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

std::string UserInterface::promptCardInfo() {
    std::string info;
    std::cout << "카드 정보를 입력하세요: ";
    std::cin >> info;
    return info;
}

std::string UserInterface::promptPrepayCode() {
    std::string code;
    std::cout << "인증코드를 입력하세요: ";
    std::cin >> code;
    return code;
}

void UserInterface::dispense(const std::string& drinkCode) {
    std::cout << "[음료 배출] 코드 " << drinkCode << " 에 해당하는 음료가 배출되었습니다.\n";
}
