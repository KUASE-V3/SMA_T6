// // tests/test_network.cpp
// //
// // �׽�Ʈ �ó�����
// // ����������������������������������������������������������������������������������������������������������������������������������������������������������������������������
// // �� No. �� �׽�Ʈ ����      �� �۽���        �� ������ �� ����                                    ��
// // ����������������������������������������������������������������������������������������������������������������������������������������������������������������������������
// // �� 1   �� ��ε�ĳ��Ʈ ��� �� T1 (��Ʈ 9000) �� T2 (��Ʈ 9001) ���� �� ����(RESP_STOCK)           ��
// // ��     �� ��ȸ ��û       ��               �� T1 ���� �� �α� ���                             ��
// // ����������������������������������������������������������������������������������������������������������������������������������������������������������������������������
// // �� 2   �� ���� ������ ��û �� T1 �� T2       �� T2 ����(REQ_PREPAY) �� ����(RESP_PREPAY)          ��
// // ��     ��               ��               �� T1 ���� �� �α� ���                             ��
// // ����������������������������������������������������������������������������������������������������������������������������������������������������������������������������

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
//     // 1) Asio I/O ���ؽ�Ʈ
//     boost::asio::io_context io;

//     // 2) OVM �ּ� ����ҿ��� ��������Ʈ �ε�
//     persistence::OvmAddressRepository repo;
//     auto endpoints = repo.getAllEndpoints();             // ��� ��ε�ĳ��Ʈ ���
//     // ID��endpoint ��
//     std::unordered_map<std::string, std::string> id_map;
//     for (const auto& id : {"T1","T2"}) {
//         id_map[id] = repo.getEndpoint(id);
//     }

//     // 3) MessageSender, MessageReceiver �ν��Ͻ�
//     network::MessageSender sender(io, endpoints, id_map);
//     // �� ���Ǳ⺰ ���ù�: T1�� 9000��, T2�� 9001�� ��Ʈ
//     unsigned short portT1 = std::stoi(repo.getEndpoint("T1").substr(repo.getEndpoint("T1").find(':')+1));
//     unsigned short portT2 = std::stoi(repo.getEndpoint("T2").substr(repo.getEndpoint("T2").find(':')+1));
//     network::MessageReceiver receiverT1(io, portT1);
//     network::MessageReceiver receiverT2(io, portT2);

//     // 4) T1���� RESP_STOCK �� RESP_PREPAY ���� �ݹ�
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

//     // 5) T2���� REQ_STOCK �� REQ_PREPAY ���� �ݹ�
//     receiverT2.subscribe(Message::Type::REQ_STOCK,
//         [&](const Message& req){
//             std::cout << "[T2] Received REQ_STOCK from " << req.src_id << "\n";
//             // RESP_STOCK ����
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
//             // RESP_PREPAY ���� (always available for test)
//             Message resp;
//             resp.msg_type = Message::Type::RESP_PREPAY;
//             resp.src_id   = "T2";
//             resp.dst_id   = req.src_id;
//             resp.msg_content = {
//                 {"item_code",    req.msg_content.at("item_code")},
//                 {"item_num",     req.msg_content.at("item_num")},
//                 {"availability", "T"}  // �׽�Ʈ�� �׻� ����
//             };
//             sender.send(resp);
//         });

//     // 6) ���ù�(���� ����) ����
//     receiverT1.start();
//     receiverT2.start();

//     // 7) io_context�� ���� ������� ����
//     std::thread io_thread([&]{ io.run(); });

//     // --- �׽�Ʈ �ó����� 1: ��ε�ĳ��Ʈ ��� ��ȸ ---
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

//     // --- �׽�Ʈ �ó����� 2: ���� ������ ��û ---
//     {
//         Message req;
//         req.msg_type   = Message::Type::REQ_PREPAY;
//         req.src_id     = "T1";
//         req.dst_id     = "T2";  // unicast to T2
//         req.msg_content = {
//             {"item_code", "05"},
//             {"item_num",  "02"},
//             {"cert_code", "ABC123"}  // �׽�Ʈ��
//         };
//         std::cout << "\n--- Scenario 2: Unicast REQ_PREPAY ---\n";
//         sender.send(req);
//         std::this_thread::sleep_for(500ms);
//     }

//     // 8) ���� �� ����
//     io.stop();
//     io_thread.join();

//     return 0;
// }
