#ifndef VENDINGMACHINE_H
#define VENDINGMACHINE_H

#include <string>
#include <utility>
namespace domain {

class VendingMachine {
    public:
    VendingMachine(const std::string& id = "T5", const std::pair<int, int>& location = {123, 123}, const std::string& port = "8080");

    std::string getId() const;

    std::pair<int, int> getLocation() const;

    std::string getPort() const;


    private:
        std::string id; 
        std::pair<int, int> location; 
        std::string port; 
    };
    
}

#endif
