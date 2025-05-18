#include "../include/domain/drink.h"
#include <string>

//기본생성자
Drink::Drink()
    : name_(""), price_(0), code_("") {}


Drink::Drink(std::string name, int price, std::string code)
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
    
    

    