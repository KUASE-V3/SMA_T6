// 모든 엔드포인트에 메시지를 브로드캐스트로 보내는 기능 테스트
#include <gtest/gtest.h>
#include "network/MessageSender.hpp"

TEST(MessageSenderTest, BroadcastMessageToAllEndpoints) {
    boost::asio::io_context io;
    std::vector<std::string> endpoints = {"127.0.0.1:9000", "127.0.0.1:9001"};
    std::unordered_map<std::string, std::string> id_map = {{"T1", "127.0.0.1:9000"}, {"T2", "127.0.0.1:9001"}};

    network::MessageSender sender(io, endpoints, id_map);

    network::Message msg;
    msg.msg_type = network::Message::Type::REQ_STOCK;
    msg.src_id = "T1";
    msg.dst_id = "0";  // broadcast
    msg.msg_content = {{"item_code", "D005"}, {"item_num", "02"}};

    EXPECT_NO_THROW(sender.send(msg));
}
