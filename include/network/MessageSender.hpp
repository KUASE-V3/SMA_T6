// include/network/MessageSender.hpp
#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <boost/asio/ip/tcp.hpp>
#include "network/message.hpp"

namespace network {

// MessageSender: 메시지를 브로드캐스트 또는 유니캐스트로 전송
// - dst_id == "0" 인 경우 endpoints_ 전체로 전송
// - 아닌 경우 id_map_을 통해 특정 endpoint로 전송
class MessageSender {
public:
    MessageSender(boost::asio::io_context& io,
                  const std::vector<std::string>& endpoints,
                  const std::unordered_map<std::string, std::string>& id_map);
    void send(const Message& msg);

private:
    void sendOne(const std::string& endpoint, const Message& msg);

    boost::asio::io_context& io_context_;
    std::vector<std::string> endpoints_;
    std::unordered_map<std::string, std::string> id_map_;
};
}