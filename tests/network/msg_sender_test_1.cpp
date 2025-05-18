#include <gtest/gtest.h>
#include "network/MessageSender.hpp"
#include "network/message.hpp"

TEST(MessageSenderTest, BroadcastSendMultipleEndpoints) {
    boost::asio::io_context io;
    std::vector<std::string> endpoints = {"127.0.0.1:9000", "127.0.0.1:9001"};
    std::unordered_map<std::string, std::string> id_map; // not used here

    network::MessageSender sender(io, endpoints, id_map);
    network::Message msg;
    msg.dst_id = "0";  // Broadcast
    msg.src_id = "T1";
    msg.msg_type = network::Message::Type::REQ_STOCK;
    msg.msg_content = {{"item_code", "D001"}, {"item_num", "01"}};

    // 단순히 예외 없이 실행되는지만 확인
    ASSERT_NO_THROW(sender.send(msg));
}
