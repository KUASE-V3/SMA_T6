#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>

#include "network/message.hpp"
#include "network/MessageReceiver.hpp"
#include "network/MessageSender.hpp"
#include "persistence/OvmAddressRepository.hpp"
#include "domain/drink.h"
#include "domain/order.h"
#include "domain/vendingMachine.h"
#include "InventoryService.hpp"
#include "PrepaymentService.hpp"
#include "ErrorService.hpp"

namespace service   { class ErrorService;      }
namespace service   { class InventoryService;  }
namespace service   { class PrepaymentService; }

namespace application {

class MessageService {
public:
    /** ctor  
     *  @param sender   ��Ʈ��ũ �۽ű�  
     *  @param receiver ��Ʈ��ũ ���ű�  
     *  @param repo     VM ID �� endpoint �����  
     *  @param err      ErrorService (string-based)  
     *  @param inv      InventoryService  
     *  @param prepay   PrepaymentService
     */
    MessageService(network::MessageSender&              sender,
                   network::MessageReceiver&            receiver,
                   const persistence::OvmAddressRepository& repo,
                   service::ErrorService&               err,
                   service::InventoryService&           inv,
                   service::PrepaymentService&          prepay);

    /* ������������������������������ UC-8 ������������������������������ */
    void broadcastStock(const domain::Drink& drink);

    /* ������������������������������ UC-16 ���������������������������� */
    void sendPrePayReq(const domain::Order& order);

    /* ������������������������������ UC-15 ���������������������������� */
    void respondPrepayReq(const domain::Order& order);

    /* ������������������������������ UC-17 ���������������������������� */
    bool validOVMStock(const std::string&   vm_id,
                       const domain::Drink& drink,
                       int                  qty = 1);

    /* ��� ���� �޽��� ��Ʈ��-����Ʈ (���� subscribe �� �б��ϹǷ� �� ��ü) */
    void onMessage(const network::Message&);

private:
    /* ���� ��� (���� �ȿ����� ���) */
    static constexpr int kBroadcastRespMax     = 7;   // 7�� ���� or
    static constexpr int kBroadcastTimeoutSec  = 30;  // 30�� ����
    static constexpr int kValidTimeoutSec      = 10;  // validOVMStock ���

    /* ������ */
    network::MessageSender&                 sender_;
    network::MessageReceiver&               receiver_;
    const persistence::OvmAddressRepository& repo_;
    service::ErrorService&                  errSvc_;
    service::InventoryService&              invSvc_;
    service::PrepaymentService&             prepaySvc_;

    /* ��ε�ĳ��Ʈ ���� ĳ�� */
    std::mutex                              resp_mtx_;
    std::vector<network::Message>           resp_cache_;
    std::condition_variable                 resp_cv_;

    /* ���� ���� */
    void handleReqStock (const network::Message& msg);
    void handleRespStock(const network::Message& msg);
    void handleReqPrepay(const network::Message& msg);
    void handleRespPrepay(const network::Message& msg);

    /* ���� VM �ĺ�/��ǥ ? �ϼ� ������ �ӽ� ��� */
    static std::string myId();                 // TODO: VendingMachine ����
    static std::pair<int,int> myCoord();       // TODO: VendingMachine ����
};

} // namespace application
