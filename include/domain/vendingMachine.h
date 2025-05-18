#ifndef VENDINGMACHINE_H
#define VENDINGMACHINE_H

#include <string>
#include <utility>
namespace domain {

class VendingMachine {
    public:
    VendingMachine(const std::string& id, const std::pair<double, double>& location);

    std::string getId() const;

    std::pair<double, double> getLocation() const;

    private:
        std::string id;
        std::pair<double, double> location;
    };
    
}

#endif
