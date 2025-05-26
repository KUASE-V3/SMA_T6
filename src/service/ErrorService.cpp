// ----- ErrorService.cpp -----
#include <iostream>
#include "service/ErrorService.hpp"
#include "service/UserProcessController.hpp"

using namespace service;

void ErrorService::logError(const std::string& msg)
{
    UserProcessController controller;
    controller.nofityError(msg);
}
