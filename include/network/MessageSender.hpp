#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <boost/asio/io_context.hpp> // Boost.Asio 사용
#include <boost/asio/ip/tcp.hpp>     // TCP 소켓 통신을 위한 Boost.Asio 헤더
#include "network/message.hpp"         // network::Message 구조체 사용

namespace network {

/**
 * @brief 네트워크를 통해 network::Message 객체를 다른 자판기로 전송하는 클래스입니다.
 * 메시지를 특정 대상(유니캐스트) 또는 모든 등록된 다른 자판기(브로드캐스트)에 전송할 수 있습니다.
 * Boost.Asio를 사용하여 TCP 소켓 통신을 수행합니다.
 */
class MessageSender {
public:
    /**
     * @brief MessageSender 생성자.
     * @param io Boost.Asio io_context 객체에 대한 참조. 비동기 작업 스케줄링에 사용됩니다.
     * @param endpoints 브로드캐스트 시 메시지를 전송할 모든 다른 자판기들의 엔드포인트(host:port 문자열) 목록.
     * @param id_map 유니캐스트 시 사용될 자판기 ID와 해당 자판기의 엔드포인트 문자열을 매핑하는 맵.
     */
    MessageSender(boost::asio::io_context& io,
                  const std::vector<std::string>& endpoints, // 브로드캐스트용 엔드포인트 목록
                  const std::unordered_map<std::string, std::string>& id_map); // 유니캐스트용 ID-엔드포인트 맵

    /**
     * @brief 주어진 network::Message 객체를 전송합니다.
     * 메시지의 dst_id가 "0"이면 브로드캐스트하고, 그렇지 않으면 해당 ID의 자판기에 유니캐스트합니다.
     * @param msg 전송할 network::Message 객체.
     */
    void send(const Message& msg);

private:
    /**
     * @brief 특정 엔드포인트로 메시지를 전송하는 내부 헬퍼 함수입니다.
     * @param endpoint 메시지를 전송할 대상의 엔드포인트 문자열 (형식: "host:port").
     * @param msg 전송할 network::Message 객체.
     * @throws std::exception TCP 연결 또는 전송 실패 시.
     */
    void sendOne(const std::string& endpoint, const Message& msg);

    boost::asio::io_context& io_context_; // Boost.Asio io_context 참조
    std::vector<std::string> endpoints_;  // 브로드캐스트 대상 엔드포인트 목록
    std::unordered_map<std::string, std::string> id_map_; // 자판기 ID별 엔드포인트 맵
};

} // namespace network
