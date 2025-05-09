#include <iostream>
#include "person.h"

int main() {
    Person p("Baik", 26);
    std::cout << p.getName() << " is " << p.getAge() << " years old." << std::endl;
    return 0;
}