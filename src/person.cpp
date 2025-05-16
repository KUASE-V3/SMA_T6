#include "person.h"

Person::Person(const std::string& name, int age)
    : name_(name), age_(age) {}

const std::string& Person::getName() const {
    return name_;
}

int Person::getAge() const {
    return age_;
}


// #include "person.h"

// Person::Person(const std::string& name, int age)
//     : name_(name)
// {
//     // age_ 멤버 변수를 초기화하지 않음 (문제!)
// }

// std::string Person::getName() const {
//     return name_;
// }

// int Person::getAge() const {
//     return age_; // 초기화되지 않은 변수 반환
// }