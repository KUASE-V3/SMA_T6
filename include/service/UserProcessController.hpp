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
#include "network/message.hpp"

class UserProcessController {
public:
    UserProcessController();
    void handleMenu();
    void handlePrepayCode();
    void handleDrinkSelection();
    void handlePayment(const bool& isPrepay);
    void nofityError(const std::string& error);
    void nearestVM(const network::Message& msg);
    void showPrepaymentCode(const std::string& prepaymentCode);
    std::string prepayFlow_UC12();

private:
    service::InventoryService inventoryService;
    UserInterface ui;
    service::ErrorService errorService;
    OrderService orderService;
    service::DistanceService distanceService;
    service::PrepaymentService prepaymentService;
};

#endif