#include "service/MessageService.hpp"
#include "network/MessageSender.hpp"
#include "network/MessageReceiver.hpp"
#include "persistence/OvmAddressRepository.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <limits>
#include <cmath>

using namespace application;
using network::Message;

/* ────────── 내부 상수 (타임아웃 등) ────────── */
namespace {
constexpr int kNetworkTimeoutMs = 500;   
const std::string kMyVmId = "T5";        // TODO:내 아이디 가져오기 
}

/*────────────────── 생성자 ──────────────────*/
MessageService::MessageService(network::MessageSender&   sender,
                               network::MessageReceiver& receiver,
                               const persistence::OvmAddressRepository& repo)
  : sender_(sender), receiver_(receiver), repo_(repo)
{
    /* 모든 수신 메시지를 헬퍼로 라우팅 */
    receiver_.subscribe(Message::Type::REQ_STOCK,
                        [this](auto& m){ handleReqStock(m); });
    receiver_.subscribe(Message::Type::RESP_STOCK,
                        [this](auto& m){ handleRespStock(m); });
    receiver_.subscribe(Message::Type::REQ_PREPAY,
                        [this](auto& m){ handleReqPrepay(m); });
    receiver_.subscribe(Message::Type::RESP_PREPAY,
                        [this](auto& m){ handleRespPrepay(m); });
}

/*────────────────── UC-8 ──────────────────*/
void MessageService::broadcastStock(const domain::Drink& drink)
{
    Message req;
    req.msg_type = Message::Type::REQ_STOCK;
    req.src_id   = kMyVmId;
    req.dst_id   = "0";                  // broadcast
    req.msg_content = {
        {"item_code", drink.code()},
        {"item_num",  "01"}              // TODO: 요청 수량 필요 시 수정
    };
    sender_.send(req);
}

/*────────────────── UC-9 ──────────────────*/
std::optional<std::string>
MessageService::getDistance(const std::vector<Message>& replies)
{
    /* TODO:
     * ① 내 좌표  : CoordinateService.myPosition()
     * ② 거리 metric: 팀 규칙(예: 맨해튼)
     * ③ tie-break : 요구서 확인
     */
    if (replies.empty()) return std::nullopt;

    std::string nearest;
    int         best     = std::numeric_limits<int>::max();

    for (const auto& m : replies) {
        int x = std::stoi(m.msg_content.at("coor_x"));
        int y = std::stoi(m.msg_content.at("coor_y"));
        // TODO: 실제 거리 계산으로 교체 거리 계산 
        int dist = std::abs(x) + std::abs(y);

        if (dist < best || (dist == best && m.src_id < nearest)) {
            best    = dist;
            nearest = m.src_id;
        }
    }
    return nearest;
}

/*────────────────── UC-16 ──────────────────*/
void MessageService::sendPrePayReq(const domain::Order& order)
{
    /* TODO: repo_.getEndpoint(order.vm_id()) 가 실패하면 ErrorService.log() */

    Message req;
    req.msg_type = Message::Type::REQ_PREPAY;
    req.src_id   = kMyVmId; 
    req.dst_id   = order.vm_id(); //이거 어떻게 가져오는지 모르겠음
    req.msg_content = {
        {"item_code", order.drinkCode()},
        {"item_num",  std::to_string(order.qty())},
        {"cert_code", order.certCode()}      // "NONE" 일 수도 있음
    };
    sender_.send(req);
}

/*────────────────── UC-15 ──────────────────*/
void MessageService::respondPrepayReq(const domain::Order& order)
{
    /* TODO:
     * ① InventoryService.reserveStock(order.drink, order.qty)
     * ② PaymentModule.generateCertCode(order)
     * ③ availability T/F 설정 후 RESP_PREPAY 송신
     */
}

/*────────────────── UC-17 ──────────────────*/
bool MessageService::validOVMStock(const std::string&   vm_id,
                                   const domain::Drink& drink,
                                   int                  qty)
{
    /* 1) 요청 송신 */
    Message req;
    req.msg_type = Message::Type::REQ_STOCK;
    req.src_id   = kMyVmId;
    req.dst_id   = vm_id;
    req.msg_content = {
        {"item_code", drink.code()},
        {"item_num",  std::to_string(qty)}
    };
    sender_.send(req);

    /* 2) TODO: condition_variable wait 로 실제 응답 대기 */
    std::this_thread::sleep_for(std::chrono::milliseconds(kNetworkTimeoutMs));

    // TODO: 응답 검사 후 true/false
    return false;
}

/*────────────────── 모든 수신 라우팅 (현재 미사용) ──────────────────*/
void MessageService::onMessage(const Message&) {}

/*────────────────── 내부 헬퍼 ──────────────────*/
void MessageService::handleReqStock(const Message& req)
{
    /* TODO:
     * InventoryService.currentQty(...)
     * CoordinateService.myPosition()
     */
    (void)req; // 컴파일러 warning 방지
}

void MessageService::handleRespStock(const Message& msg)
{
    std::lock_guard<std::mutex> lk(resp_mtx_);
    resp_cache_.push_back(msg);
    resp_cv_.notify_one();          // TODO: wait 구현 시 사용
}

void MessageService::handleReqPrepay(const Message& msg)
{
    /* TODO: msg → Order 변환 후 respondPrepayReq 호출 */
    (void)msg;
}

void MessageService::handleRespPrepay(const Message& msg)
{
    /* TODO: PaymentFlowManager.onPrepayResult(msg) */
    (void)msg;
}
