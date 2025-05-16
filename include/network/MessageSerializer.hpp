#pragma once

#include <string>
#include <stdexcept>
#include "network/message.hpp"

class MessageSerializer {
    public :
    // 직렬화 : 메시지 -> json
        static std::string toJson(const Message& msg);
    // 역직렬화 : json -> 메시지
        static Message fromJson(const std::string& jsonStr);
};

