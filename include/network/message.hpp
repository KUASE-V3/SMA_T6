#pragma once

#include <string>
#include <unordered_map>
namespace network {
//메시지 구조체 정의
struct Message{
    enum class Type{
        REQ_STOCK = 0,
        RESP_STOCK = 1,
        REQ_PREPAY = 2,
        RESP_PREPAY = 3,
    };

    Type msg_type{}; // 메시지 타입 기본값 설정(생성시 따로 설정해야함)
    std::string src_id = "T5"; 
    std::string dst_id = "0"; // 0 : 브로드캐스트
    std::unordered_map<std::string, std::string> msg_content; // 메시지 타입에 따른 내용

};
}