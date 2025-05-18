
#include "domain/vendingMachine.h"
#include <cmath>

using namespace domain;

// ex) ?ƒ?„±? ?˜¸ì¶? : VendingMachine vm("001", std::make_pair(37.5665, 126.9780)); // ?„œ?š¸?‹œì²? ì¢Œí‘œ
VendingMachine::VendingMachine(const std::string& id, const std::pair<double, double>& location)
    : id(id), location(location) {}

std::string VendingMachine::getId() const {
    return id;
}

std::pair<double, double> VendingMachine::getLocation() const {
    return location;
}

/*          ê±°ë¦¬ê³„ì‚° ?˜ˆ?‹œ
double VendingMachine::calculateDistance(const VendingMachine& other) const {
    double dx = location.first - other.location.first;
    double dy = location.second - other.location.second;
    return std::sqrt(dx * dx + dy * dy);        //ê³„ì‚°??? ?•Œ?•„?„œ ?‘?„±
}
*/


