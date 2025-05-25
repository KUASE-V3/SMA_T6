#include "presentation/UserInterface.hpp"
#include <iostream>
#include "service/UserProcessController.hpp"


void UserInterface::displayMainMenu() {
    std::cout << "===== �옄�뙋湲� 硫붾돱 =====\n";
    std::cout << "1. �쓬猷� 紐⑸줉 議고쉶\n2. �쓬猷� �꽑�깮\n3. �옱怨� �솗�씤\n4. 醫낅즺\n5. �씤利앹퐫�뱶 �엯�젰\n踰덊샇瑜� �꽑�깮�븯�꽭�슂: ";
}

void UserInterface::displayList(const std::vector<std::pair<std::string, int>>& list) {
    std::cout << "[�쓬猷� 紐⑸줉]\n";
    for (const auto& drink : list) {
        std::cout << "�씠由�: " << drink.first << ", 媛�寃�: " << drink.second << "�썝\n";
    }
}

void UserInterface::show_error_message(const std::string& error) {
    std::cerr << "[�뿉�윭 硫붿떆吏�] " << error << std::endl;
}

void UserInterface::display_Error(const std::string& error) {
    std::cerr << "[�몴�떆 �삤瑜�] " << error << std::endl;
}

void UserInterface::displayMessage(const std::string& msg) {
    std::cout << msg << std::endl;
}

void UserInterface::promptCardInfo() {


    UserProcessController controller;
    // handlePayment �샇異�
    controller.handlePayment(false); //UI단에서 선결제와 일반결제를 구분하지 못함 -> 이거 구조 
}

std::string UserInterface::promptPrepayCode() {
    std::string code;
    std::cout << "선결제 코드를 입력하세요: ";
    std::cin >> code;
    return code;
}

void UserInterface::dispense(const std::string& drinkCode) {
    std::cout << "[�쓬猷� 諛곗텧] 肄붾뱶 " << drinkCode << " �뿉 �빐�떦�븯�뒗 �쓬猷뚭�� 諛곗텧�릺�뿀�뒿�땲�떎.\n";
}


bool UserInterface::promptPrepayConsent() {
   //"[�꽑寃곗젣 �룞�쓽] �꽑寃곗젣 �룞�쓽 �뿬遺�瑜� �엯�젰�븯�꽭�슂 (y/n) yes -> true 諛섑솚 
    char consent;
    std::cout << "�꽑寃곗젣 �룞�쓽 �뿬遺�瑜� �엯�젰�븯�꽭�슂 (y/n): ";
    std::cin >> consent;
    return (consent == 'y' || consent == 'Y');
    }

void UserInterface::showText(const std::string& prepaymentCode) {
    std::cout << "�꽑寃곗젣 肄붾뱶: " << prepaymentCode << std::endl;
}

void UserInterface::display_SomeText(const std::string& text) {
    std::cout << text << std::endl;
}