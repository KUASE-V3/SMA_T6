#include "service/UserProcessController.hpp"
#include <iostream>
#include <string>
#include <algorithm>

int main() {
    std::cout << "스마트 자판기 시스템을 시작합니다...\n";

    UserProcessController controller;

    while (true) {
        std::cout << "========= Main Menu =========\n";
        std::cout << "1. 음료 목록 보기\n";
        std::cout << "2. 음료 선택 및 결제\n";
        std::cout << "3. 선결제 코드 입력\n";
        std::cout << "0. 종료\n";
        std::cout << "=============================\n";
        std::cout << "선택: ";

        std::string input;
        std::cin >> input;

        // 숫자인지 검증
        bool isNumber = !input.empty() && std::all_of(input.begin(), input.end(), ::isdigit);
        if (!isNumber) {
            std::cout << "잘못된 입력입니다. 숫자를 입력해주세요.\n";
            continue;
        }

        int choice = std::stoi(input);

        switch (choice) {
            case 1:
                controller.handleMenu();
                break;
            case 2:
                controller.handleDrinkSelection();
                break;
            case 3:
                controller.handlePrepayCode();
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