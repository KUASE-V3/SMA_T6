// // tests/test_network.cpp
// //
// // 테스트 시나리오
// // ┌─────┬───────────────┬──────────────┬───────────────────────────────────────────────┐
// // │ No. │ 테스트 종류      │ 송신자        │ 수신자 및 동작                                    │
// // ├─────┼───────────────┼──────────────┼───────────────────────────────────────────────┤
// // │ 1   │ 브로드캐스트 재고 │ T1 (포트 9000) │ T2 (포트 9001) 수신 → 응답(RESP_STOCK)           │
// // │     │ 조회 요청       │               │ T1 수신 → 로그 출력                             │
// // ├─────┼───────────────┼──────────────┼───────────────────────────────────────────────┤
// // │ 2   │ 단일 선결제 요청 │ T1 → T2       │ T2 수신(REQ_PREPAY) → 응답(RESP_PREPAY)          │
// // │     │               │               │ T1 수신 → 로그 출력                             │
// // └─────┴───────────────┴──────────────┴───────────────────────────────────────────────┘

// #include <iostream>
// #include <thread>
// #include <chrono>
// #include <vector>
// #include <unordered_map>

// #include <boost/asio.hpp>

// #include "network/message.hpp"
// #include "network/MessageSerializer.hpp"
// #include "network/MessageSender.hpp"
// #include "network/MessageReceiver.hpp"
// #include "persistence/OvmAddressRepository.hpp"

// using namespace std::chrono_literals;
// using boost::asio::ip::tcp;

// int main() {
//     // 1) Asio I/O 컨텍스트
//     boost::asio::io_context io;

//     // 2) OVM 주소 저장소에서 엔드포인트 로드
//     persistence::OvmAddressRepository repo;
//     auto endpoints = repo.getAllEndpoints();             // 모든 브로드캐스트 대상
//     // ID→endpoint 맵
//     std::unordered_map<std::string, std::string> id_map;
//     for (const auto& id : {"T1","T2"}) {
//         id_map[id] = repo.getEndpoint(id);
//     }

//     // 3) MessageSender, MessageReceiver 인스턴스
//     network::MessageSender sender(io, endpoints, id_map);
//     // 각 자판기별 리시버: T1은 9000번, T2는 9001번 포트
//     unsigned short portT1 = std::stoi(repo.getEndpoint("T1").substr(repo.getEndpoint("T1").find(':')+1));
//     unsigned short portT2 = std::stoi(repo.getEndpoint("T2").substr(repo.getEndpoint("T2").find(':')+1));
//     network::MessageReceiver receiverT1(io, portT1);
//     network::MessageReceiver receiverT2(io, portT2);

//     // 4) T1에서 RESP_STOCK 및 RESP_PREPAY 수신 콜백
//     receiverT1.subscribe(Message::Type::RESP_STOCK,
//         [](const Message& resp){
//             std::cout << "[T1] Received RESP_STOCK from " << resp.src_id
//                       << " item_code=" << resp.msg_content.at("item_code")
//                       << ", item_num=" << resp.msg_content.at("item_num")
//                       << ", coord=(" << resp.msg_content.at("coor_x")
//                       << "," << resp.msg_content.at("coor_y") << ")\n";
//         });
//     receiverT1.subscribe(Message::Type::RESP_PREPAY,
//         [](const Message& resp){
//             std::cout << "[T1] Received RESP_PREPAY from " << resp.src_id
//                       << " item_code=" << resp.msg_content.at("item_code")
//                       << ", item_num=" << resp.msg_content.at("item_num")
//                       << ", availability=" << resp.msg_content.at("availability") << "\n";
//         });

//     // 5) T2에서 REQ_STOCK 및 REQ_PREPAY 수신 콜백
//     receiverT2.subscribe(Message::Type::REQ_STOCK,
//         [&](const Message& req){
//             std::cout << "[T2] Received REQ_STOCK from " << req.src_id << "\n";
//             // RESP_STOCK 응답
//             Message resp;
//             resp.msg_type = Message::Type::RESP_STOCK;
//             resp.src_id   = "T2";
//             resp.dst_id   = req.src_id;
//             resp.msg_content = {
//                 {"item_code", req.msg_content.at("item_code")},
//                 {"item_num",  req.msg_content.at("item_num")},
//                 {"coor_x",    "10"},
//                 {"coor_y",    "20"}
//             };
//             sender.send(resp);
//         });
//     receiverT2.subscribe(Message::Type::REQ_PREPAY,
//         [&](const Message& req){
//             std::cout << "[T2] Received REQ_PREPAY from " << req.src_id << "\n";
//             // RESP_PREPAY 응답 (always available for test)
//             Message resp;
//             resp.msg_type = Message::Type::RESP_PREPAY;
//             resp.src_id   = "T2";
//             resp.dst_id   = req.src_id;
//             resp.msg_content = {
//                 {"item_code",    req.msg_content.at("item_code")},
//                 {"item_num",     req.msg_content.at("item_num")},
//                 {"availability", "T"}  // 테스트용 항상 성공
//             };
//             sender.send(resp);
//         });

//     // 6) 리시버(수신 루프) 시작
//     receiverT1.start();
//     receiverT2.start();

//     // 7) io_context를 별도 스레드로 실행
//     std::thread io_thread([&]{ io.run(); });

//     // --- 테스트 시나리오 1: 브로드캐스트 재고 조회 ---
//     {
//         Message req;
//         req.msg_type   = Message::Type::REQ_STOCK;
//         req.src_id     = "T1";
//         req.dst_id     = "0";  // broadcast
//         req.msg_content = {{"item_code","05"}, {"item_num","02"}};
//         std::cout << "\n--- Scenario 1: Broadcast REQ_STOCK ---\n";
//         sender.send(req);
//         std::this_thread::sleep_for(500ms);
//     }

//     // --- 테스트 시나리오 2: 단일 선결제 요청 ---
//     {
//         Message req;
//         req.msg_type   = Message::Type::REQ_PREPAY;
//         req.src_id     = "T1";
//         req.dst_id     = "T2";  // unicast to T2
//         req.msg_content = {
//             {"item_code", "05"},
//             {"item_num",  "02"},
//             {"cert_code", "ABC123"}  // 테스트용
//         };
//         std::cout << "\n--- Scenario 2: Unicast REQ_PREPAY ---\n";
//         sender.send(req);
//         std::this_thread::sleep_for(500ms);
//     }

//     // 8) 정리 및 종료
//     io.stop();
//     io_thread.join();

//     return 0;
// }
