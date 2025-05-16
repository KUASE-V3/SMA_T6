#ifndef DRINK_H
#define DRINK_H

#include <string>


class Drink {
    public:
        Drink();
        
        Drink(std::string name, int price, std::string code);

        std::string getName() const;
        int getPrice() const;
        std::string getCode() const;

    private:
        std:: string  name_;
        int price_;
        std:: string code_;
};

#endif