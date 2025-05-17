// ----- ErrorService.cpp -----
#include <iostream>
#include "service/ErrorService.hpp"
#include <iostream>

using namespace service;

std::string ErrorService::logError(const std::string& msg)
{
    std::cerr << "[ErrorService] " << msg << '\n';
    return msg;
}
