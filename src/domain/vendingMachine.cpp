
#include "domain/vendingMachine.h"
#include <cmath>

using namespace domain;

// 초기화시 : id, location을 "T5", {123, 123}으로 설정합니다.
VendingMachine::VendingMachine(const std::string& id, const std::pair<int, int>& location, const std::string& port)
    : id(id), location(location), port(port){}

std::string VendingMachine::getId() const {
    return id;
}

std::pair<int, int> VendingMachine::getLocation() const {
    return location;
}

std::string VendingMachine::getPort() const {
    return port;
}

/*          嫄곕━怨꾩궛 �삁�떆
double VendingMachine::calculateDistance(const VendingMachine& other) const {
    double dx = location.first - other.location.first;
    double dy = location.second - other.location.second;
    return std::sqrt(dx * dx + dy * dy);        //怨꾩궛��� �븣�븘�꽌 �옉�꽦
}
*/


