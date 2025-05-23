#include "service/UserProcessController.hpp"
#include <iostream>
#include "persistence/inventoryRepository.cpp"

int main() {

    // Initialize the UserProcessController
    UserProcessController controller;
    
    while (true) {
        std::cout << "\n========= Main Menu =========\n";
        std::cout << "1. �쓬猷� 紐⑸줉 蹂닿린\n";
        std::cout << "2. �쓬猷� �꽑�깮 諛� 寃곗젣\n";
        std::cout << "3. �꽑寃곗젣 肄붾뱶 �엯�젰\n";
        std::cout << "0. 醫낅즺\n";
        std::cout << "=============================\n";
        std::cout << "�꽑�깮: ";

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
                std::cout << "�봽濡쒓렇�옩�쓣 醫낅즺�빀�땲�떎.\n";
                return 0;
            default:
                std::cout << "�옒紐삳맂 �엯�젰�엯�땲�떎.\n";
        }
    }

    return 0;
}
