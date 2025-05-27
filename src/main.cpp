#include "service/UserProcessController.hpp"
#include <iostream>
#include "persistence/inventoryRepository.cpp"

int main() {

    // Initialize the UserProcessController
    UserProcessController controller;
    
    while (true) {
        // Display the main menu
        controller.handleMenu(); // UC1
        controller.handleDrinkSelection(); // UC3
        //controller.handlePrepayCode(); // UC14


    return 0;
}
}