#pragma once

#include <string>
#include <unordered_map>

namespace network {
struct Message{
    enum class Type{
        REQ_STOCK = 0,
        RESP_STOCK = 1,
        REQ_PREPAY = 2,
        RESP_PREPAY = 3,
    };

    Type msg_type{}; 
    std::string src_id = "T6"; 
    std::string dst_id = "0"; // 0일 때는 브로드캐스트
    std::unordered_map<std::string, std::string> msg_content; 

};

}
