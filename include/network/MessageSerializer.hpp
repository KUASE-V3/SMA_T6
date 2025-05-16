#pragma once

#include <string>
#include <stdexcept>
#include "network/message.hpp"

class MessageSerializer {
    public :
    // ����ȭ : �޽��� -> json
        static std::string toJson(const Message& msg);
    // ������ȭ : json -> �޽���
        static Message fromJson(const std::string& jsonStr);
};

