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

/* �������������������� ���� ��� (Ÿ�Ӿƿ� ��) �������������������� */
namespace {
constexpr int kNetworkTimeoutMs = 500;   
const std::string kMyVmId = "T5";        // TODO:�� ���̵� �������� 
}

/*������������������������������������ ������ ������������������������������������*/
MessageService::MessageService(network::MessageSender&   sender,
                               network::MessageReceiver& receiver,
                               const persistence::OvmAddressRepository& repo)
  : sender_(sender), receiver_(receiver), repo_(repo)
{
    /* ��� ���� �޽����� ���۷� ����� */
    receiver_.subscribe(Message::Type::REQ_STOCK,
                        [this](auto& m){ handleReqStock(m); });
    receiver_.subscribe(Message::Type::RESP_STOCK,
                        [this](auto& m){ handleRespStock(m); });
    receiver_.subscribe(Message::Type::REQ_PREPAY,
                        [this](auto& m){ handleReqPrepay(m); });
    receiver_.subscribe(Message::Type::RESP_PREPAY,
                        [this](auto& m){ handleRespPrepay(m); });
}

/*������������������������������������ UC-8 ������������������������������������*/
void MessageService::broadcastStock(const domain::Drink& drink)
{
    Message req;
    req.msg_type = Message::Type::REQ_STOCK;
    req.src_id   = kMyVmId;
    req.dst_id   = "0";                  // broadcast
    req.msg_content = {
        {"item_code", drink.code()},
        {"item_num",  "01"}              // TODO: ��û ���� �ʿ� �� ����
    };
    sender_.send(req);
}

/*������������������������������������ UC-9 ������������������������������������*/
std::optional<std::string>
MessageService::getDistance(const std::vector<Message>& replies)
{
    /* TODO:
     * �� �� ��ǥ  : CoordinateService.myPosition()
     * �� �Ÿ� metric: �� ��Ģ(��: ����ư)
     * �� tie-break : �䱸�� Ȯ��
     */
    if (replies.empty()) return std::nullopt;

    std::string nearest;
    int         best     = std::numeric_limits<int>::max();

    for (const auto& m : replies) {
        int x = std::stoi(m.msg_content.at("coor_x"));
        int y = std::stoi(m.msg_content.at("coor_y"));
        // TODO: ���� �Ÿ� ������� ��ü �Ÿ� ��� 
        int dist = std::abs(x) + std::abs(y);

        if (dist < best || (dist == best && m.src_id < nearest)) {
            best    = dist;
            nearest = m.src_id;
        }
    }
    return nearest;
}

/*������������������������������������ UC-16 ������������������������������������*/
void MessageService::sendPrePayReq(const domain::Order& order)
{
    /* TODO: repo_.getEndpoint(order.vm_id()) �� �����ϸ� ErrorService.log() */

    Message req;
    req.msg_type = Message::Type::REQ_PREPAY;
    req.src_id   = kMyVmId; 
    req.dst_id   = order.vm_id(); //�̰� ��� ���������� �𸣰���
    req.msg_content = {
        {"item_code", order.drinkCode()},
        {"item_num",  std::to_string(order.qty())},
        {"cert_code", order.certCode()}      // "NONE" �� ���� ����
    };
    sender_.send(req);
}

/*������������������������������������ UC-15 ������������������������������������*/
void MessageService::respondPrepayReq(const domain::Order& order)
{
    /* TODO:
     * �� InventoryService.reserveStock(order.drink, order.qty)
     * �� PaymentModule.generateCertCode(order)
     * �� availability T/F ���� �� RESP_PREPAY �۽�
     */
}

/*������������������������������������ UC-17 ������������������������������������*/
bool MessageService::validOVMStock(const std::string&   vm_id,
                                   const domain::Drink& drink,
                                   int                  qty)
{
    /* 1) ��û �۽� */
    Message req;
    req.msg_type = Message::Type::REQ_STOCK;
    req.src_id   = kMyVmId;
    req.dst_id   = vm_id;
    req.msg_content = {
        {"item_code", drink.code()},
        {"item_num",  std::to_string(qty)}
    };
    sender_.send(req);

    /* 2) TODO: condition_variable wait �� ���� ���� ��� */
    std::this_thread::sleep_for(std::chrono::milliseconds(kNetworkTimeoutMs));

    // TODO: ���� �˻� �� true/false
    return false;
}

/*������������������������������������ ��� ���� ����� (���� �̻��) ������������������������������������*/
void MessageService::onMessage(const Message&) {}

/*������������������������������������ ���� ���� ������������������������������������*/
void MessageService::handleReqStock(const Message& req)
{
    /* TODO:
     * InventoryService.currentQty(...)
     * CoordinateService.myPosition()
     */
    (void)req; // �����Ϸ� warning ����
}

void MessageService::handleRespStock(const Message& msg)
{
    std::lock_guard<std::mutex> lk(resp_mtx_);
    resp_cache_.push_back(msg);
    resp_cv_.notify_one();          // TODO: wait ���� �� ���
}

void MessageService::handleReqPrepay(const Message& msg)
{
    /* TODO: msg �� Order ��ȯ �� respondPrepayReq ȣ�� */
    (void)msg;
}

void MessageService::handleRespPrepay(const Message& msg)
{
    /* TODO: PaymentFlowManager.onPrepayResult(msg) */
    (void)msg;
}
