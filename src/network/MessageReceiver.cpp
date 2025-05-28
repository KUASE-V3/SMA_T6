#include "network/MessageReceiver.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>    
#include <boost/asio/streambuf.hpp>    
#include <boost/asio/write.hpp>
#include <iostream>

using boost::asio::ip::tcp;

namespace network {
MessageReceiver::MessageReceiver(boost::asio::io_context& io, unsigned short port)
    : io_context_(io), port_(port),
      acceptor_(io, tcp::endpoint(tcp::v4(), port)) {}

void MessageReceiver::subscribe(Message::Type type, Handler handler) {
    handlers_[type] = std::move(handler);
}

void MessageReceiver::start() {
    doAccept();
}

void MessageReceiver::doAccept() {
    std::cout << " Accepting connections on port " << port_ << std::endl;
    auto sock = std::make_shared<tcp::socket>(io_context_);
    acceptor_.async_accept(*sock, [this, sock](boost::system::error_code ec) {
        if (!ec) {
            doRead(sock);
        }
        doAccept();
    });
}

void MessageReceiver::doRead(std::shared_ptr<tcp::socket> sock) {
    auto buf = std::make_shared<boost::asio::streambuf>();
    boost::asio::async_read_until(*sock, *buf, '\n',
        [this, sock, buf](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                std::istream is(buf.get());
                std::string line;
                std::getline(is, line);
                try {
                    auto msg = MessageSerializer::fromJson(line);
                    auto it = handlers_.find(msg.msg_type);
                    if (it != handlers_.end()) {
                        it->second(msg);
                    }
                } catch (const std::exception& e) {
                    std::cerr << "[Receiver] parse error: " << e.what() << "\n";
                }
                doRead(sock);
            }
        });
}
}