#pragma once
#include <string>

namespace service {

class ErrorService {
public:
    std::string logError(const std::string& msg);
};

} // namespace service
