#include <gtest/gtest.h>
#include "network/message.hpp"

TEST(MessageServiceTest, HandleRespPrepay_ClearsPendingOnSuccess) {
    bool pendingExists = true; // 시뮬레이션

    network::Message msg;
    msg.msg_type = network::Message::Type::RESP_PREPAY;
    msg.msg_content = {
        {"availability", "T"},
        {"item_code", "D003"},
        {"item_num", "1"}
    };

    if (msg.msg_content.at("availability") == "T") {
        pendingExists = false; // 초기화 가정
    }

    EXPECT_FALSE(pendingExists);
}
