// Ư�� �޽��� Ÿ�Կ� ���� �ڵ鷯�� ���� ��ϵǴ��� �׽�Ʈ
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include "network/MessageReceiver.hpp"

TEST(MessageReceiverTest, SubscribeHandler) {
    boost::asio::io_context io;
    unsigned short port = 9002;
    network::MessageReceiver receiver(io, port);

    bool called = false;

    receiver.subscribe(network::Message::Type::REQ_STOCK, [&](const network::Message& msg) {
        called = true;
    });

    // �ڵ鷯 ��� Ȯ�ο� - ���� ������ �� ���� ������ ȣ�� ���� ������ �������� ����
    EXPECT_NO_THROW(receiver.start());
}
