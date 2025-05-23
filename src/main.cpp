#include "service/UserProcessController.hpp"
#include <iostream>

int main() {

    // Initialize the UserProcessController
    UserProcessController controller;
    
    while (true) {
        std::cout << "\n========= Main Menu =========\n";
        std::cout << "1. 음료 목록 보기\n";
        std::cout << "2. 음료 선택 및 결제\n";
        std::cout << "3. 선결제 코드 입력\n";
        std::cout << "0. 종료\n";
        std::cout << "=============================\n";
        std::cout << "선택: ";

        int choice;
        std::cin >> choice;

        switch (choice) {
            case 1:
                controller.handleMenu(); // UC1
                break;
            case 3:
                controller.handlePrepayCode(); // UC14
                break;
            case 0:
                std::cout << "프로그램을 종료합니다.\n";
                return 0;
            default:
                std::cout << "잘못된 입력입니다.\n";
        }
    }

    return 0;
}
