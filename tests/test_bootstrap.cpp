// tests/test_bootstrap.cpp
//
// 목적: MessageSender / MessageReceiver / MessageService 배선이
//       빌드·링킹만 제대로 되는지 확인하는 간단 엔드투엔드 스모크 테스트.
//
// ? 진짜 도메인 객체(Drink·Order)는 아직 없으므로
//    여기서만 사용하는 임시 Stub 구조체로 대체했다.
//    (팀원 파트가 완료되면 교체/삭제)

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

    // Receiver @9000 (실제로 받는 자판기 역할)
    network::MessageReceiver receiver(io, 9000);
    receiver.subscribe(network::Message::Type::REQ_STOCK, [](const network::Message& msg) {
        std::cout << "[VM@9000] Got REQ_STOCK\n";
    });
    receiver.start();

    // IO 루프 스레드
    std::thread io_thread([&]{ io.run(); });

    // 송신자 설정
    auto endpoints = repo.getAllEndpoints();
    std::unordered_map<std::string,std::string> id_map{
        {"T1", repo.getEndpoint("T1")},
        {"T2", repo.getEndpoint("T2")}
    };
    network::MessageSender sender(io, endpoints, id_map);

    // 서비스 객체 (Sender + Receiver + Repo)
    application::MessageService svc(sender, receiver, repo);

    // ── 테스트 시나리오 ──
    {
        std::cout << ">>> broadcastStock()\n";
        svc.broadcastStock(domain::Drink{"05"});
        std::this_thread::sleep_for(500ms);
    }
    {
        std::cout << ">>> sendPrePayReq()\n";
        svc.sendPrePayReq(domain::Order("T1", domain::Drink("커피"), 2));
        std::this_thread::sleep_for(500ms);
    }

    io.stop();
    io_thread.join();
}
