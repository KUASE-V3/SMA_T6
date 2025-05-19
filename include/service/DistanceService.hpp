#ifndef DISTANCE_SERVICE_HPP
#define DISTANCE_SERVICE_HPP

#include <vector>
#include <utility>
#include "domain/vendingMachine.h"  // VendingMachine 직접 사용

class DistanceService {
public:
    // 가장 가까운 자판기의 좌표 반환
    std::pair<double, double> findNearest(double userX, double userY, const std::vector<domain::VendingMachine>& machines);
};

#endif // DISTANCE_SERVICE_HPP
