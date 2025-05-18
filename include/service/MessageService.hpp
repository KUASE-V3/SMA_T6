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
     *  @param sender   네트워크 송신기  
     *  @param receiver 네트워크 수신기  
     *  @param repo     VM ID ↔ endpoint 저장소  
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

    /* ─────────────── UC-8 ─────────────── */
    void broadcastStock(const domain::Drink& drink);

    /* ─────────────── UC-16 ────────────── */
    void sendPrePayReq(const domain::Order& order);

    /* ─────────────── UC-15 ────────────── */
    void respondPrepayReq(const domain::Order& order);

    /* ─────────────── UC-17 ────────────── */
    bool validOVMStock(const std::string&   vm_id,
                       const domain::Drink& drink,
                       int                  qty = 1);

    /* 모든 수신 메시지 엔트리-포인트 (현재 subscribe 로 분기하므로 빈 몸체) */
    void onMessage(const network::Message&);

private:
    /* 내부 상수 (파일 안에서만 사용) */
    static constexpr int kBroadcastRespMax     = 7;   // 7개 응답 or
    static constexpr int kBroadcastTimeoutSec  = 30;  // 30초 종료
    static constexpr int kValidTimeoutSec      = 10;  // validOVMStock 대기

    /* 의존성 */
    network::MessageSender&                 sender_;
    network::MessageReceiver&               receiver_;
    const persistence::OvmAddressRepository& repo_;
    service::ErrorService&                  errSvc_;
    service::InventoryService&              invSvc_;
    service::PrepaymentService&             prepaySvc_;

    /* 브로드캐스트 응답 캐시 */
    std::mutex                              resp_mtx_;
    std::vector<network::Message>           resp_cache_;
    std::condition_variable                 resp_cv_;

    /* 내부 헬퍼 */
    void handleReqStock (const network::Message& msg);
    void handleRespStock(const network::Message& msg);
    void handleReqPrepay(const network::Message& msg);
    void handleRespPrepay(const network::Message& msg);

    /* 현재 VM 식별/좌표 ? 완성 전까지 임시 상수 */
    static std::string myId();                 // TODO: VendingMachine 연동
    static std::pair<int,int> myCoord();       // TODO: VendingMachine 연동
};

} // namespace application
