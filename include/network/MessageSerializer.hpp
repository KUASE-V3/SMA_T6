#pragma once
#include <string>
#include "message.hpp"

namespace network {

/** Message <-> JSON 문자열 직렬화 유틸 (RapidJSON 기반) */
class MessageSerializer {
public:
    static std::string toJson(const Message& msg);     // TODO: 구현
    static Message fromJson(const std::string& json);// TODO: 구현
};

} // namespace network
