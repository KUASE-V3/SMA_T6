// // MessageService::broadcastStock() 호출 시 send가 정상 수행되는지 확인
// #include <gtest/gtest.h>
// #include "network/MessageSender.hpp"
// #include "network/MessageReceiver.hpp"
// #include "service/MessageService.hpp"
// #include "service/ErrorService.hpp"
// #include "service/InventoryService.hpp"
// #include "service/PrepaymentService.hpp"
// #include "domain/drink.h"
// #include "domain/inventory.h"
// #include "persistence/OvmAddressRepository.hpp"

// TEST(MessageServiceTest, BroadcastStockTest) {
//     boost::asio::io_context io;

//     // 가짜 repo, 에러서비스, 서비스
//     persistence::OvmAddressRepository repo;
//     service::ErrorService errSvc;
//     service::InventoryService invSvc;
//     service::PrepaymentService prepaySvc;
//     domain::Drink d("콜라", 1500, "D005");
//     domain::inventory drink(d, 10);

//     std::vector<std::string> eps = {"127.0.0.1:9000"};
//     std::unordered_map<std::string, std::string> id_map = {{"T1", "127.0.0.1:9000"}};
//     network::MessageSender sender(io, eps, id_map);
//     network::MessageReceiver receiver(io, 9000);

//     application::MessageService msgSvc(sender, receiver, repo, errSvc, invSvc, prepaySvc, drink);

//     EXPECT_NO_THROW(msgSvc.broadcastStock(d));
// }
