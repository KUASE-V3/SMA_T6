#pragma once

#include <string>
#include <unordered_map>
namespace network { 
//�޽��� ����ü ���� 
struct Message{
    enum class Type{
        REQ_STOCK = 0,
        RESP_STOCK = 1,
        REQ_PREPAY = 2,
        RESP_PREPAY = 3,
    };

    Type msg_type{}; // �޽��� Ÿ�� �⺻�� ����(������ ���� �����ؾ���)
    std::string src_id = "T5"; 
    std::string dst_id = "0"; // 0 : ��ε�ĳ��Ʈ
    std::unordered_map<std::string, std::string> msg_content; // �޽��� Ÿ�Կ� ���� ����

};
}