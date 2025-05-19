#include "service/DistanceService.hpp"
#include <cmath>
#include <limits>
#include <iostream>

using namespace domain;

std::pair<double, double> DistanceService::findNearest(double userX, double userY, const std::vector<VendingMachine>& machines) {
    double minDistance = std::numeric_limits<double>::max();
    std::pair<double, double> location;

    for (const auto& vm : machines) {
        auto [x, y] = vm.getLocation();  // 반환형: pair<int, int>
        double dx = userX - x;
        double dy = userY - y;
        double distance = std::sqrt(dx * dx + dy * dy);

        if (distance < minDistance || (distance == minDistance && vm.getId() < machines.front().getId())) {
            minDistance = distance;
            location = {static_cast<double>(x), static_cast<double>(y)};  // int → double
        }
    }

    std::cerr << "[거리 계산] 가장 가까운 자판기 location: (" << location.first << ", " << location.second << "), 거리: " << minDistance << std::endl;
    return location;
}