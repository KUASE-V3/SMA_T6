// tests/test_bootstrap.cpp
//
// ����: MessageSender / MessageReceiver / MessageService �輱��
//       ���塤��ŷ�� ����� �Ǵ��� Ȯ���ϴ� ���� ���������� ����ũ �׽�Ʈ.
//
// ? ��¥ ������ ��ü(Drink��Order)�� ���� �����Ƿ�
//    ���⼭�� ����ϴ� �ӽ� Stub ����ü�� ��ü�ߴ�.
//    (���� ��Ʈ�� �Ϸ�Ǹ� ��ü/����)

#include <iostream>
#include <thread>
#include <chrono>
#include <boost/asio.hpp>

#include "persistence/OvmAddressRepository.hpp"
#include "network/MessageSender.hpp"
#include "network/MessageReceiver.hpp"
#include "service/MessageService.hpp"

using namespace std::chrono_literals;
using boost::asio::ip::tcp;

int main() {
    boost::asio::io_context io;
    persistence::OvmAddressRepository repo;

    // Receiver @9000 (������ �޴� ���Ǳ� ����)
    network::MessageReceiver receiver(io, 9000);
    receiver.subscribe(network::Message::Type::REQ_STOCK, [](const network::Message& msg) {
        std::cout << "[VM@9000] Got REQ_STOCK\n";
    });
    receiver.start();

    // IO ���� ������
    std::thread io_thread([&]{ io.run(); });

    // �۽��� ����
    auto endpoints = repo.getAllEndpoints();
    std::unordered_map<std::string,std::string> id_map{
        {"T1", repo.getEndpoint("T1")},
        {"T2", repo.getEndpoint("T2")}
    };
    network::MessageSender sender(io, endpoints, id_map);

    // ���� ��ü (Sender + Receiver + Repo)
    application::MessageService svc(sender, receiver, repo);

    // ���� �׽�Ʈ �ó����� ����
    {
        std::cout << ">>> broadcastStock()\n";
        svc.broadcastStock(domain::Drink{"05"});
        std::this_thread::sleep_for(500ms);
    }
    {
        std::cout << ">>> sendPrePayReq()\n";
        svc.sendPrePayReq(domain::Order("T1", domain::Drink("Ŀ��"), 2));
        std::this_thread::sleep_for(500ms);
    }

    io.stop();
    io_thread.join();
}
