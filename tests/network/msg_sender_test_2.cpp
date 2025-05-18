#include <gtest/gtest.h>
#include "network/MessageSender.hpp"
#include "network/message.hpp"

TEST(MessageSenderTest, UnicastSendToTarget) {
    boost::asio::io_context io;
    std::vector<std::string> endpoints = {};
    std::unordered_map<std::string, std::string> id_map = {{"T2", "127.0.0.1:9001"}};

    network::MessageSender sender(io, endpoints, id_map);
    network::Message msg;
    msg.dst_id = "T2";  // Unicast
    msg.src_id = "T1";
    msg.msg_type = network::Message::Type::REQ_PREPAY;
    msg.msg_content = {{"item_code", "D002"}, {"item_num", "01"}, {"cert_code", "XYZ123"}};

    ASSERT_NO_THROW(sender.send(msg));
}
