// DistanceService.cpp
// ------------------------------
#include "service/DistanceService.hpp"
#include <iostream>

namespace service {

void DistanceService::getDistance(const std::vector<std::string>& list) {
    std::cout << "[DistanceService] ê±°ë¦¬ ê³„ì‚° ?š”ì²??¨: ";
    for (const auto& item : list) std::cout << item << " ";
    std::cout << std::endl;
}

}
