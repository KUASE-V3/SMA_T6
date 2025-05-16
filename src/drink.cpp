#include "drink.h"
#include <string>



Drink::Drink(std::string name, double price, std::string code)
    : name_(name), price_(price), code_(code) {}


    std::string Drink::getName() const {
        return name_;
    }
    
    int Drink:: getPrice() const {
        return price_;
    }
    
    std::string Drink::getCode() const {
        return code_;
    }
    
    

    