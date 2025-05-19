// DistanceService.hpp
// ------------------------------
#ifndef DISTANCE_SERVICE_HPP
#define DISTANCE_SERVICE_HPP

#include <vector>
#include <string>

namespace service {

class DistanceService {
public:
    void getDistance(const std::vector<std::string>& list);
};
}
#endif
