// UserProcessController.hpp
// ------------------------------
#ifndef USER_PROCESS_CONTROLLER_HPP
#define USER_PROCESS_CONTROLLER_HPP

#include "InventoryService.hpp"
#include "UserInterface.hpp"
#include "ErrorService.hpp"
#include "OrderService.hpp"
#include "DistanceService.hpp"

class UserProcessController {
public:
    UserProcessController();
    void handleMenu();
    void handlePrepayCode();
    void handleDrinkSelection();
    void handlePayment(const std::string& cardInfo);

private:
    InventoryService inventoryService;
    UserInterface ui;
    ErrorService errorService;
    OrderService orderService;
    DistanceService distanceService;
};

#endif