#include <gtest/gtest.h>
#include "service/MessageService.hpp"
#include "domain/drink.h"

TEST(MessageServiceTest, BroadcastStockCreatesCorrectMessage) {
    domain::Drink drink("ÄÝ¶ó", 1500, "D001");

    network::Message msg;
    msg.msg_type = network::Message::Type::REQ_STOCK;
    msg.dst_id = "0";
    msg.msg_content = {{"item_code", drink.getCode()}, {"item_num", "01"}};

    EXPECT_EQ(msg.msg_content.at("item_code"), "D001");
    EXPECT_EQ(msg.dst_id, "0");
}
