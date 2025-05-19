// 특정 메시지 타입에 대한 핸들러가 정상 등록되는지 테스트
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

    // 핸들러 등록 확인용 - 내부 구조를 볼 수는 없지만 호출 구조 오류가 없는지만 본다
    EXPECT_NO_THROW(receiver.start());
}
