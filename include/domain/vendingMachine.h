#ifndef VENDINGMACHINE_H
#define VENDINGMACHINE_H

#include <string>

class VendingMachine {
    public:
    VendingMachine(const std::string& id, const double& location);

    std::string getId() const;
    double getLocation() const;


    private:
        std::string id;
        double location;
    };
    
    #endif