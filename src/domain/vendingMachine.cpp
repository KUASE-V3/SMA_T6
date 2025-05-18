
#include "domain/vendingMachine.h"
#include <cmath>

using namespace domain;

// ex) ?��?��?�� ?���? : VendingMachine vm("001", std::make_pair(37.5665, 126.9780)); // ?��?��?���? 좌표
VendingMachine::VendingMachine(const std::string& id, const std::pair<double, double>& location)
    : id(id), location(location) {}

std::string VendingMachine::getId() const {
    return id;
}

std::pair<double, double> VendingMachine::getLocation() const {
    return location;
}

/*          거리계산 ?��?��
double VendingMachine::calculateDistance(const VendingMachine& other) const {
    double dx = location.first - other.location.first;
    double dy = location.second - other.location.second;
    return std::sqrt(dx * dx + dy * dy);        //계산??? ?��?��?�� ?��?��
}
*/


