#ifndef VENDINGMACHINE_H
#define VENDINGMACHINE_H

#include <string>

namespace domain {
class VendingMachine {
    public:
    VendingMachine(const std::string& id, const std::string& location);

    std::string getId() const;
    std::string getLocation() const;


    private:
        std::string id;
        std::string location;
    };
    
}
    #endif