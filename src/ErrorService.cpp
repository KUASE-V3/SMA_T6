// ----- ErrorService.cpp -----
#include "ErrorService.h"
#include <iostream>

std::string ErrorService::logError(const std::string& message) {
    std::cerr << "[오류 기록됨] " << message << std::endl;
    return message;
}
