#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <boost/asio/ip/tcp.hpp>
#include "network/message.hpp"

namespace network {

// MessageSender: �޽����� ��ε�ĳ��Ʈ �Ǵ� ����ĳ��Ʈ�� ����
// - dst_id == "0" �� ��� endpoints_ ��ü�� ����
// - �ƴ� ��� id_map_�� ���� Ư�� endpoint�� ����
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