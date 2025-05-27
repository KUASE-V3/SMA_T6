#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <type_traits>
#include <boost/asio/ip/tcp.hpp>
#include "network/message.hpp"
#include "network/MessageSerializer.hpp"


namespace network {
// Enum class�� ���� �ؽ� ����
struct EnumClassHash {
    template <typename T>
    size_t operator()(T t) const noexcept {
        return std::hash<std::underlying_type_t<T>>{}(
            static_cast<std::underlying_type_t<T>>(t));
    }
};

// MessageReceiver: TCP ������ ���� ������ JSON �޽����� ����
// - msg_type�� handler�� ����Ͽ� �б� ó��
class MessageReceiver {
    friend class test_MessageReceiver_subscribe;  

public:
    using Handler = std::function<void(const Message&)>;

    MessageReceiver(boost::asio::io_context& io, unsigned short port);
    void subscribe(Message::Type type, Handler handler);
    void start();

private:
    void doAccept();
    void doRead(std::shared_ptr<boost::asio::ip::tcp::socket> sock);

    boost::asio::io_context& io_context_;
    unsigned short port_;
    boost::asio::ip::tcp::acceptor acceptor_;  // acceptor�� ����� ����
    std::unordered_map<Message::Type, Handler, EnumClassHash> handlers_;
};
}