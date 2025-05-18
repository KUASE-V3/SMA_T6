#include "service/MessageService.hpp"

#include "domain/vendingMachine.h"
#include "service/ErrorService.hpp"
#include "service/InventoryService.hpp"
#include "service/PrepaymentService.hpp"


#include <iostream>
#include <algorithm>
#include <cmath>
#include <thread>

using application::MessageService;

/* ────────────── 정적 HELPER ────────────── */
std::string MessageService::myId()
{
    domain::VendingMachine vm("001",);
    return vm.getId();
}
std::pair<int,int> MessageService::myCoord()
{
    // NOTE: 도메인 완성 전까지 (0,0)
    return {0,0};
}

/* ────────────── ctor ────────────── */
MessageService::MessageService(network::MessageSender&   sender,
                               network::MessageReceiver& receiver,
                               const persistence::OvmAddressRepository& repo,
                               service::ErrorService&    err,
                               service::InventoryService& inv,
                               service::PrepaymentService& prepay)
  : sender_(sender)
  , receiver_(receiver)
  , repo_(repo)
  , errSvc_(err)
  , invSvc_(inv)
  , prepaySvc_(prepay)
{
    using Type = network::Message::Type;
    receiver_.subscribe(Type::REQ_STOCK,   [this](auto&m){ handleReqStock(m);   });
    receiver_.subscribe(Type::RESP_STOCK,  [this](auto&m){ handleRespStock(m);  });
    receiver_.subscribe(Type::REQ_PREPAY,  [this](auto&m){ handleReqPrepay(m);  });
    receiver_.subscribe(Type::RESP_PREPAY, [this](auto&m){ handleRespPrepay(m); });
}

/* ────────────── UC-8 ────────────── */
void MessageService::broadcastStock(const domain::Drink& drink)
{
    network::Message req;
    req.msg_type = network::Message::Type::REQ_STOCK;
    req.src_id   = domain::VendingMachine::getId(); // 내 자판기 ID
    req.dst_id   = "0";                     // broadcast
    req.msg_content = {
        {"item_code", drink.getCode()},
        {"item_num",  "01"}
    };

    try { sender_.send(req); }
    catch(const std::exception& e){
        errSvc_.logError(std::string("broadcastStock send() failed: ")+e.what());
    }
}

/* ────────────── UC-16 ────────────── */
void MessageService::sendPrePayReq(const domain::Order& order)
{
    network::Message msg;
    msg.msg_type = network::Message::Type::REQ_PREPAY;
    msg.src_id   = myId();
    msg.dst_id   = order.vmId();            // 대상 VM
    msg.msg_content = {
        {"item_code", order.drink().getCode()},
        {"item_num",  std::to_string(order.quantity())},
        {"cert_code", order.certCode()}
    };

    try { sender_.send(msg); }
    catch(const std::exception& e){
        errSvc_.logError(std::string("sendPrePayReq send() failed: ")+e.what());
    }
}

/* ────────────── UC-15 (수신측) ────────────── */
void MessageService::respondPrepayReq(const domain::Order& order)
{
    // ① 선결제 서비스에 코드 발행 / 재고 확보
    if(!prepaySvc_.isValid(order.certCode())){
        errSvc_.logError("respondPrepayReq: invalid code");
        return;
    }
    // TODO: InventoryService.reserve(...) 등 실제 확보 로직

    /* 성공 가정 ? 응답 전송 */
    network::Message resp;
    resp.msg_type = network::Message::Type::RESP_PREPAY;
    resp.src_id   = myId();
    resp.dst_id   = order.vmId();
    resp.msg_content = {
        {"availability","T"},
        {"item_code", order.drink().getCode()}
    };

    try { sender_.send(resp); }
    catch(const std::exception& e){
        errSvc_.logError(std::string("RESP_PREPAY send() failed: ")+e.what());
    }
}

/* ────────────── UC-17 ────────────── */
bool MessageService::validOVMStock(const std::string& vm_id,
                                   const domain::Drink& drink,
                                   int qty)
{
    network::Message req;
    req.msg_type = network::Message::Type::REQ_STOCK;
    req.src_id   = myId();
    req.dst_id   = vm_id;
    req.msg_content = {
        {"item_code", drink.getCode()},
        {"item_num",  std::to_string(qty)}
    };

    try { sender_.send(req); }
    catch(const std::exception& e){
        errSvc_.logError(std::string("validOVMStock send() failed: ")+e.what());
        return false;
    }

    /* 대기 */
    std::unique_lock lk(resp_mtx_);
    bool got = resp_cv_.wait_for(lk,
                 std::chrono::seconds(kValidTimeoutSec),
                 [&]{ return std::any_of(resp_cache_.begin(), resp_cache_.end(),
                                  [&](auto& m){ return m.src_id == vm_id; }); });

    if(!got) return false;

    auto it = std::find_if(resp_cache_.begin(), resp_cache_.end(),
                           [&](auto& m){ return m.src_id == vm_id; });

    bool available = (it != resp_cache_.end()) &&
                     (it->msg_content.at("item_num") != "0");

    resp_cache_.clear();
    return available;
}

/* ─────────── 핸들러들 ─────────── */
void MessageService::handleReqStock(const network::Message& msg)
{
    // 재고 확인 후 RESP_STOCK 전송
    bool empty = invSvc_.getSaleValid(msg.msg_content.at("item_code")) == false;

    network::Message resp;
    resp.msg_type = network::Message::Type::RESP_STOCK;
    resp.src_id   = myId();
    resp.dst_id   = msg.src_id;
    resp.msg_content = {
        {"item_code", msg.msg_content.at("item_code")},
        {"item_num",  empty ? "0" : msg.msg_content.at("item_num")},
        {"coor_x",    std::to_string(myCoord().first)},
        {"coor_y",    std::to_string(myCoord().second)}
    };
    try { sender_.send(resp); }
    catch(const std::exception& e){
        errSvc_.logError(std::string("handleReqStock send() failed: ")+e.what());
    }
}

void MessageService::handleRespStock(const network::Message& msg)
{
    {
        std::lock_guard lg(resp_mtx_);
        resp_cache_.push_back(msg);
        if(resp_cache_.size() >= kBroadcastRespMax)
            resp_cv_.notify_all();
    }
}

void MessageService::handleReqPrepay(const network::Message& msg)
{
    // TODO: msg → Order 변환
    // respondPrepayReq(order);
}

void MessageService::handleRespPrepay(const network::Message& msg)
{
    if(msg.msg_content.at("availability") == "F")
        errSvc_.logError("RESP_PREPAY: availability == F");
}

void MessageService::onMessage(const network::Message&) { /* unused */ }
