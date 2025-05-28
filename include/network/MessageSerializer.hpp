#pragma once
#include <string>
#include "message.hpp"

namespace network {

/** Message <-> JSON (RapidJSON ) */
class MessageSerializer {
public:
    static std::string toJson(const Message& msg);     
    static Message fromJson(const std::string& json);
};

} // namespace network
