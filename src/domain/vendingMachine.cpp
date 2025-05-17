#include "vendingMachine.h"

// ex) 생성자 호출 : VendingMachine vm("001", "Seoul");
VendingMachine::VendingMachine(const std::string& id, const std::string& location)
    : id(id), location(location) {}


    std::string VendingMachine::getId() const {
        return id;
    }
    
    std::string VendingMachine::getLocation() const {
        return location;
    }    

