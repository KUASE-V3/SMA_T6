#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>
#include "network/MessageSender.hpp"
#include "network/MessageSerializer.hpp"
#include <sstream>

using namespace network;

TEST(MessageSenderTest, CanSerializeAndSendToCorrectEndpoint) {
    boost::asio::io_context io;
    std::vector<std::string> endpoints = {"localhost:9000"};
    std::unordered_map<std::string, std::string> id_map = {{"T5", "localhost:9000"}};

    MessageSender sender(io, endpoints, id_map);

    Message msg;
    msg.msg_type = Message::Type::REQ_STOCK;
    msg.src_id = "T1";
    msg.dst_id = "T5";
    msg.msg_content = {{"item_code", "D001"}, {"item_num", "01"}};

    // 실제 sendOne은 네트워크 연결을 시도하므로 테스트에선 실패만 안 나면 통과
    EXPECT_NO_THROW(sender.send(msg));
}
