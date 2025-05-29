#pragma once

#include <functional>      
#include <string>          
#include <unordered_map>    
#include <type_traits>      
#include <memory>           

#include <boost/asio/io_context.hpp> 
#include <boost/asio/ip/tcp.hpp>     // Boost.Asio TCP 관련 타입

#include "network/message.hpp"         
#include "network/MessageSerializer.hpp" 

namespace network {

/**
 * @brief enum class 타입을 위한 사용자 정의 해시 함수 객체입니다.
 * 이 해시 함수를 통해 network::Message::Type과 같은 enum class 타입을
 * std::unordered_map의 키로 사용할 수 있습니다.
 */
struct EnumClassHash {
    template <typename T>
    size_t operator()(T t) const noexcept {
        return std::hash<std::underlying_type_t<T>>{}(
            static_cast<std::underlying_type_t<T>>(t)
        );
    }
};

/**
 * @brief 들어오는 TCP 연결을 처리하고 수신된 메시지를 가공합니다.
 * 지정된 포트에서 수신 대기하고, 연결을 수락하며, 비동기적으로 데이터를 읽습니다.
 * 수신된 JSON 데이터를 network::Message 객체로 역직렬화하고,
 * 메시지 유형에 따라 등록된 핸들러로 전달합니다.
 */
class MessageReceiver {
    // 테스트 목적으로 private 멤버에 접근해야 할 경우를 위한 friend 선언
    friend class test_MessageReceiver_subscribe;

public:
    /**
     * @brief 수신된 메시지를 처리하기 위한 콜백 함수 타입 정의입니다.
     * 수신된 network::Message에 대한 const 참조를 인자로 받습니다.
     */
    using Handler = std::function<void(const Message&)>;

    /**
     * @brief MessageReceiver 생성자입니다.
     * 지정된 포트에서 수신 대기하도록 acceptor를 초기화합니다.
     * @param io Boost.Asio io_context 객체에 대한 참조. 비동기 I/O 작업 스케줄링에 사용됩니다.
     * @param port 수신기가 들어오는 연결을 수신 대기할 포트 번호입니다.
     */
    MessageReceiver(boost::asio::io_context& io, unsigned short port);

    /**
     * @brief 특정 메시지 타입에 대한 핸들러 함수를 구독(등록)합니다.
     * 해당 타입의 메시지가 수신되면 등록된 핸들러가 호출됩니다.
     * @param type 구독할 network::Message::Type.
     * @param handler 해당 'type'의 메시지가 수신되었을 때 호출될 Handler 함수.
     */
    void subscribe(Message::Type type, Handler handler);

    /**
     * @brief 메시지 수신기를 시작합니다.
     * 이 메소드는 들어오는 연결을 수락하는 프로세스를 시작합니다.
     * 필요한 모든 핸들러가 구독된 후에 호출되어야 합니다.
     */
    void start();

private:
    /**
     * @brief 새로운 들어오는 TCP 연결을 비동기적으로 수락합니다.
     * 연결이 성공적으로 이루어지면, doRead()를 호출하여 소켓으로부터 데이터 읽기를 시작합니다.
     * 그 후, 다음 연결을 수락하도록 자신을 다시 스케줄링합니다.
     */
    void doAccept();

    /**
     * @brief 주어진 TCP 소켓으로부터 개행 문자를 만날 때까지 비동기적으로 데이터를 읽습니다.
     * 수신된 데이터는 JSON에서 Message 객체로 역직렬화된 후, 적절한 핸들러가 호출됩니다.
     * 메시지 처리 후 (또는 오류 발생 시), 동일한 소켓에서 다음 메시지를 읽도록 자신을 다시 스케줄링합니다.
     * @param sock 데이터를 읽어올 TCP 소켓에 대한 std::shared_ptr.
     */
    void doRead(std::shared_ptr<boost::asio::ip::tcp::socket> sock);

    boost::asio::io_context& io_context_; ///< Boost.Asio io_context에 대한 참조.
    unsigned short port_;                 ///< 이 수신기가 수신 대기하는 포트 번호.
    boost::asio::ip::tcp::acceptor acceptor_; ///< 들어오는 TCP 연결을 위한 Boost.Asio acceptor.
    
    // 메시지 타입으로 키가 지정된 핸들러들을 저장합니다. Message::Type에 EnumClassHash를 사용합니다.
    std::unordered_map<Message::Type, Handler, EnumClassHash> handlers_;
};

} // namespace network
