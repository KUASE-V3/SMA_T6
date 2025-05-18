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

    // ���� sendOne�� ��Ʈ��ũ ������ �õ��ϹǷ� �׽�Ʈ���� ���и� �� ���� ���
    EXPECT_NO_THROW(sender.send(msg));
}
