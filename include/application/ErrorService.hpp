// include/ErrorService.hpp
#ifndef ERROR_SERVICE_HPP
#define ERROR_SERVICE_HPP

#include <string>

class ErrorService {
public:
    static std::string logError(const std::string& message);
};

#endif // ERROR_SERVICE_HPP
