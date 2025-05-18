#include "vendingMachine.h"

// ex) 생성자 호출 : VendingMachine vm("001", "Seoul == 5");
VendingMachine::VendingMachine(const std::string& id, const double& location)
    : id(id), location(location) {}


    std::string VendingMachine::getId() const {
        return id;
    }
    
    double VendingMachine::getLocation() const {
        return location;
    }    

