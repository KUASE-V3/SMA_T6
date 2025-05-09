#include <iostream>
#include "person.h"

int main() {
    Person p("HyunSu", 30);
    std::cout << p.getName() << " is " << p.getAge() << " years old." << std::endl;
    return 0;


}