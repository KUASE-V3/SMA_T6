#include "network/MessageSender.hpp"
#include "network/MessageSerializer.hpp"
#include <boost/asio/connect.hpp>
#include <boost/asio/write.hpp>
#include <iostream>

using boost::asio::ip::tcp;

namespace network {
MessageSender::MessageSender(boost::asio::io_context& io,
                             const std::vector<std::string>& endpoints,
                             const std::unordered_map<std::string, std::string>& id_map)
    : io_context_(io), endpoints_(endpoints), id_map_(id_map) {}

void MessageSender::send(const Message& msg) {
    if (msg.dst_id == "0") { // broadcast
        for (auto& ep : endpoints_) {
            sendOne(ep, msg);

        }
    } else { // unicast
        auto it = id_map_.find(msg.dst_id);
        if (it != id_map_.end()) {
            sendOne(it->second, msg);
        }
    }
}

void MessageSender::sendOne(const std::string& endpoint, const Message& msg) {
    auto pos = endpoint.find(':');
    std::string host = endpoint.substr(0, pos);
    unsigned short port = static_cast<unsigned short>(std::stoi(endpoint.substr(pos + 1)));

    tcp::resolver resolver(io_context_);
    auto eps = resolver.resolve(host, std::to_string(port));
    tcp::socket socket(io_context_);
    boost::asio::connect(socket, eps);
    
    std::string data = MessageSerializer::toJson(msg) + "\n";
    boost::asio::write(socket, boost::asio::buffer(data));
}

}