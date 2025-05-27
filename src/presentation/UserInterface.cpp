#include "presentation/UserInterface.hpp"
#include <iostream>
#include "service/UserProcessController.hpp"


void UserInterface::displayMainMenu() {
    std::cout << "UserInterface : Main Menu\n";
}

void UserInterface::displayList(const std::vector<std::pair<std::string, int>>& list) {
    std::cout << "UserInterface : displayList\n";
    for (const auto& drink : list) {
        std::cout << "음료수: " << drink.first << ", 가격: " << drink.second << "원\n";
    }
}

void UserInterface::show_error_message(const std::string& error) {
    std::cerr << "[UserInterface : show_error_message] " << error << std::endl;
}

void UserInterface::display_Error(const std::string& error) {
    std::cerr << "[UserInterface : display_Error] " << error << std::endl;
}

void UserInterface::displayMessage(const std::string& msg) {
    std::cout << msg << std::endl;
}

void UserInterface::promptCardInfo() {


    UserProcessController controller;
    controller.handlePayment(false); 
}

std::string UserInterface::promptPrepayCode() {
    std::string code;
    std::cout << "drinkCode를 입력하세요: ";
    std::cin >> code;
    return code;
}

void UserInterface::dispense(const std::string& drinkCode) {
    std::cout << "[UserInterface : dispence] " << drinkCode;
}


bool UserInterface::promptPrepayConsent() {
    char consent;
    std::cout << "선결제에 동의하십니까? (y/n): ";
    std::cin >> consent;
    return (consent == 'y' || consent == 'Y');
    }

void UserInterface::showText(const std::string& prepaymentCode) {
    std::cout << "UserInterface : showText" << prepaymentCode << std::endl;
}

void UserInterface::display_SomeText(const std::string& text) {
    std::cout << text << std::endl;
}