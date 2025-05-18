// DistanceService.cpp
// ------------------------------
#include "service/DistanceService.hpp"
#include <iostream>

void DistanceService::getDistance(const std::vector<std::string>& list) {
    std::cout << "[DistanceService] 거리 계산 요청됨: ";
    for (const auto& item : list) std::cout << item << " ";
    std::cout << std::endl;
}
