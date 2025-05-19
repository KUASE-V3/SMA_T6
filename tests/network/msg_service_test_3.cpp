#include <gtest/gtest.h>
#include "domain/drink.h"
#include "domain/order.h"
#include "network/message.hpp"

TEST(MessageServiceTest, PrepayResponseSuccessMessage) {
    domain::Drink drink("∏ÛΩ∫≈Õ", 2500, "D005");
    domain::Order order("T2", drink, 1, "CERT123", "Pending");

    network::Message msg;
    msg.msg_type = network::Message::Type::RESP_PREPAY;
    msg.dst_id = order.vmId();
    msg.msg_content = {
        {"availability", "T"},
        {"item_num", std::to_string(order.quantity())},
        {"item_code", drink.getCode()}
    };

    EXPECT_EQ(msg.msg_content.at("availability"), "T");
    EXPECT_EQ(msg.msg_content.at("item_code"), "D005");
}
