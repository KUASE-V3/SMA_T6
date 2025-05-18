#include <gtest/gtest.h>
#include "network/message.hpp"

TEST(MessageServiceTest, PrepayResponseFailMessage) {
    network::Message msg;
    msg.msg_type = network::Message::Type::RESP_PREPAY;
    msg.msg_content = {
        {"availability", "F"},
        {"item_num", "2"},
        {"item_code", "D009"}
    };

    EXPECT_EQ(msg.msg_content.at("availability"), "F");
    EXPECT_EQ(msg.msg_content.at("item_code"), "D009");
}
