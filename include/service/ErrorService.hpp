#pragma once
#include <string>

namespace service {

class ErrorService {
public:
    void logError(const std::string& msg);
};

} // namespace service
