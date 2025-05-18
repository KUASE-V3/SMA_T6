// UserProcessController.hpp
// ------------------------------
#ifndef USER_PROCESS_CONTROLLER_HPP
#define USER_PROCESS_CONTROLLER_HPP

#include "service/InventoryService.hpp"
#include "presentation/UserInterface.hpp"
#include "service/ErrorService.hpp"
#include "OrderService.hpp"
#include "DistanceService.hpp"
#include "domain/order.h"
#include "service/PrepaymentService.hpp" 

class UserProcessController {
public:
    UserProcessController();
    void handleMenu();
    void handlePrepayCode();
    void handleDrinkSelection();
    void handlePayment(const std::string& cardInfo, domain::Order& order);

private:
    service::InventoryService inventoryService;
    UserInterface ui;
    service::ErrorService errorService;
    OrderService orderService;
    DistanceService distanceService;
    service::PrepaymentService prepaymentService;
};

#endif