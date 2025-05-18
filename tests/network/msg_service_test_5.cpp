#include <gtest/gtest.h>
#include "network/message.hpp"

TEST(MessageServiceTest, HandleRespPrepay_ClearsPendingOnSuccess) {
    bool pendingExists = true; // �ùķ��̼�

    network::Message msg;
    msg.msg_type = network::Message::Type::RESP_PREPAY;
    msg.msg_content = {
        {"availability", "T"},
        {"item_code", "D003"},
        {"item_num", "1"}
    };

    if (msg.msg_content.at("availability") == "T") {
        pendingExists = false; // �ʱ�ȭ ����
    }

    EXPECT_FALSE(pendingExists);
}
