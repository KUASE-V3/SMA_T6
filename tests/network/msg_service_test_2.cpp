#include <gtest/gtest.h>
#include "domain/drink.h"
#include "domain/order.h"
#include "network/message.hpp"

TEST(MessageServiceTest, PrepayRequestMessageFields) {
    domain::Drink drink("ªÁ¿Ã¥Ÿ", 1500, "D002");
    domain::Order order("T2", drink, 2, "ABC123", "Pending");

    network::Message msg;
    msg.msg_type = network::Message::Type::REQ_PREPAY;
    msg.dst_id = order.vmId();
    msg.msg_content = {
        {"item_code", order.drink().getCode()},
        {"item_num", std::to_string(order.quantity())},
        {"cert_code", order.certCode()}
    };

    EXPECT_EQ(msg.msg_content.at("item_code"), "D002");
    EXPECT_EQ(msg.msg_content.at("cert_code"), "ABC123");
    EXPECT_EQ(msg.dst_id, "T2");
}
