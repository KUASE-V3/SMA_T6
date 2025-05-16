#include "service/MessageService.hpp"
#include "network/MessageSender.hpp"
#include "network/MessageReceiver.hpp"
#include "persistence/OvmAddressRepository.hpp"
#include "domain/Drink.hpp" 
#include <iostream>
#include <thread>        

using application::MessageService;
namespace {

/// ������ Ÿ�Ӿƿ� ���� ���� condition_variable �� ���� ����
constexpr int kNetworkTimeoutMs = 500;

}

MessageService::MessageService(network::MessageSender&   sender,
                               network::MessageReceiver& receiver,
                               const persistence::OvmAddressRepository& repo)
  : sender_(sender)
  , receiver_(receiver)
  , repo_   (repo)
{
    /* Receiver �� Service ����� */
    receiver_.subscribe(network::Message::Type::REQ_STOCK,
                        [this](auto& m){ handleReqStock(m); });
    receiver_.subscribe(network::Message::Type::RESP_STOCK,
                        [this](auto& m){ handleRespStock(m); });
    receiver_.subscribe(network::Message::Type::REQ_PREPAY,
                        [this](auto& m){ handleReqPrepay(m); });
    receiver_.subscribe(network::Message::Type::RESP_PREPAY,
                        [this](auto& m){ handleRespPrepay(m); });
}

/* �������������������������������������� UC-8 �������������������������������������� */
void MessageService::broadcastStock(const domain::Drink& drink)
{
    network::Message req;
    req.msg_type = network::Message::Type::REQ_STOCK;
    req.src_id   = "T5";                // TODO: �� VM ID ����
    req.dst_id   = "0";                 // ��ε�ĳ��Ʈ
    req.msg_content = {
        {"item_code", drink.code()},
        {"item_num",  "01"}
    };
    sender_.send(req);
}

/* �������������������������������������� UC-9 (����) �������������������������������������� */
void MessageService::getDistance(const std::vector<network::Message>&)
{
    // TODO: DistanceService::findNearest(...) ȣ��
}

/* �������������������������������������� UC-16 �������������������������������������� */
void MessageService::sendPrePayReq(const domain::Order& order)
{
    auto dst   = order.vm_id();                // Order �� ���� ��� VM
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

/* �������������������������������������� UC-15 (���� ��) �������������������������������������� */
void MessageService::respondPrepayReq(const domain::Order& /*order*/)
{
    // TODO: ���� ��� Ȯ�� & PrepayCodeRepository.save(...)
}

/* �������������������������������������� UC-17 �������������������������������������� */
bool MessageService::validOVMStock(const std::string& id,
                                   const domain::Drink& drink,
                                   int qty)
{
    /* 1) ��û ���� */
    network::Message req;
    req.msg_type = network::Message::Type::REQ_STOCK;
    req.src_id   = "T5";
    req.dst_id   = id;
    req.msg_content = {
        {"item_code", drink.code()},
        {"item_num",  std::to_string(qty)}
    };
    sender_.send(req);

    /* 2) ���� ��� ? TODO: condition_variable �� ���� wait */
    std::this_thread::sleep_for(std::chrono::milliseconds(kNetworkTimeoutMs));

    /* ����: �׻� true */
    return true;
}

/* �������������������������������������� ���� ����� �������������������������������������� */
void MessageService::onMessage(const network::Message&) {
    /* ���� subscribe() �� ���� �б��ϹǷ� �� �Լ� */
}

/* �������������������������������������� ���� ���� (��� TODO) �������������������������������������� */
void MessageService::handleReqStock (const network::Message&) { /* TODO */ }
void MessageService::handleRespStock(const network::Message&) { /* TODO */ }
void MessageService::handleReqPrepay(const network::Message&) { /* TODO */ }
void MessageService::handleRespPrepay(const network::Message&) { /* TODO */ }
