#include "service/MessageService.hpp"
#include "network/MessageSender.hpp"
#include "network/MessageReceiver.hpp"
#include "persistence/OvmAddressRepository.hpp"
#include "domain/Drink.hpp" 
#include <iostream>
#include <thread>        

using application::MessageService;
namespace {

/// 응답대기 타임아웃 등은 추후 condition_variable 로 구현 예정
constexpr int kNetworkTimeoutMs = 500;

}

MessageService::MessageService(network::MessageSender&   sender,
                               network::MessageReceiver& receiver,
                               const persistence::OvmAddressRepository& repo)
  : sender_(sender)
  , receiver_(receiver)
  , repo_   (repo)
{
    /* Receiver → Service 라우팅 */
    receiver_.subscribe(network::Message::Type::REQ_STOCK,
                        [this](auto& m){ handleReqStock(m); });
    receiver_.subscribe(network::Message::Type::RESP_STOCK,
                        [this](auto& m){ handleRespStock(m); });
    receiver_.subscribe(network::Message::Type::REQ_PREPAY,
                        [this](auto& m){ handleReqPrepay(m); });
    receiver_.subscribe(network::Message::Type::RESP_PREPAY,
                        [this](auto& m){ handleRespPrepay(m); });
}

/* ─────────────────── UC-8 ─────────────────── */
void MessageService::broadcastStock(const domain::Drink& drink)
{
    network::Message req;
    req.msg_type = network::Message::Type::REQ_STOCK;
    req.src_id   = "T5";                // TODO: 내 VM ID 주입
    req.dst_id   = "0";                 // 브로드캐스트
    req.msg_content = {
        {"item_code", drink.code()},
        {"item_num",  "01"}
    };
    sender_.send(req);
}

/* ─────────────────── UC-9 (스텁) ─────────────────── */
void MessageService::getDistance(const std::vector<network::Message>&)
{
    // TODO: DistanceService::findNearest(...) 호출
}

/* ─────────────────── UC-16 ─────────────────── */
void MessageService::sendPrePayReq(const domain::Order& order)
{
    auto dst   = order.vm_id();                // Order 가 가진 대상 VM
    auto ep_it = repo_.getEndpoint(dst);

    if (ep_it.empty()) {
        // TODO: ErrorService.log_error(...)
        return;
    }

    network::Message req;
    req.msg_type = network::Message::Type::REQ_PREPAY;
    req.src_id   = "T5";
    req.dst_id   = dst;
    req.msg_content = {
        {"item_code", order.drinkCode()},
        {"item_num",  std::to_string(order.qty())},
        {"cert_code", order.certCode()}
    };
    sender_.send(req);
}

/* ─────────────────── UC-15 (수신 측) ─────────────────── */
void MessageService::respondPrepayReq(const domain::Order& /*order*/)
{
    // TODO: 실제 재고 확보 & PrepayCodeRepository.save(...)
}

/* ─────────────────── UC-17 ─────────────────── */
bool MessageService::validOVMStock(const std::string& id,
                                   const domain::Drink& drink,
                                   int qty)
{
    /* 1) 요청 전송 */
    network::Message req;
    req.msg_type = network::Message::Type::REQ_STOCK;
    req.src_id   = "T5";
    req.dst_id   = id;
    req.msg_content = {
        {"item_code", drink.code()},
        {"item_num",  std::to_string(qty)}
    };
    sender_.send(req);

    /* 2) 응답 대기 ? TODO: condition_variable 로 실제 wait */
    std::this_thread::sleep_for(std::chrono::milliseconds(kNetworkTimeoutMs));

    /* 스텁: 항상 true */
    return true;
}

/* ─────────────────── 수신 라우팅 ─────────────────── */
void MessageService::onMessage(const network::Message&) {
    /* 현재 subscribe() 가 직접 분기하므로 빈 함수 */
}

/* ─────────────────── 내부 헬퍼 (모두 TODO) ─────────────────── */
void MessageService::handleReqStock (const network::Message&) { /* TODO */ }
void MessageService::handleRespStock(const network::Message&) { /* TODO */ }
void MessageService::handleReqPrepay(const network::Message&) { /* TODO */ }
void MessageService::handleRespPrepay(const network::Message&) { /* TODO */ }
