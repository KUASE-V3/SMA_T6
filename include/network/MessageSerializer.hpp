#pragma once
#include <string>
#include "message.hpp"

namespace network {

/** Message <-> JSON ���ڿ� ����ȭ ��ƿ (RapidJSON ���) */
class MessageSerializer {
public:
    static std::string toJson(const Message& msg);     // TODO: ����
    static Message fromJson(const std::string& json);// TODO: ����
};

} // namespace network
