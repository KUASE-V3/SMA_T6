#pragma once

#include <string>
#include "network/message.hpp" // network::Message 구조체 사용을 위해 포함

namespace network {

/**
 * @brief network::Message 객체와 JSON 문자열 간의 직렬화(serialization) 및
 * 역직렬화(deserialization)를 담당하는 유틸리티 클래스입니다.
 * RapidJSON 라이브러리를 사용하여 JSON 처리를 수행합니다.
 * 모든 메소드는 정적(static)으로 제공됩니다.
 */
class MessageSerializer {
public:
    /**
     * @brief network::Message 객체를 JSON 형식의 문자열로 변환합니다.
     * PFR 문서의 표1~4에 정의된 메시지 포맷을 따릅니다.
     * @param msg 직렬화할 network::Message 객체.
     * @return JSON 형식으로 직렬화된 문자열.
     */
    static std::string toJson(const Message& msg);

    /**
     * @brief JSON 형식의 문자열을 network::Message 객체로 변환합니다.
     * PFR 문서의 표1~4에 정의된 메시지 포맷을 따릅니다.
     * @param json 역직렬화할 JSON 형식의 문자열.
     * @return 역직렬화된 network::Message 객체.
     * @throws std::runtime_error (또는 RapidJSON 관련 예외) JSON 파싱 실패 시.
     */
    static Message fromJson(const std::string& json);
};

} // namespace network
