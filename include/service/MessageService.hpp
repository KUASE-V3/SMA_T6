#pragma once


#include "network/message.hpp"
#include "network/MessageSender.hpp"
#include "network/MessageReceiver.hpp"

#include "persistence/OvmAddressRepository.hpp"
#include "service/ErrorService.hpp"
#include "service/InventoryService.hpp"
#include "service/PrepaymentService.hpp"

#include "domain/drink.h"
#include "domain/order.h"
#include "domain/inventory.h"

#include <vector>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <future>
#include <chrono>

namespace application {

class MessageService {

public:
    /* 타임아웃 / 응답 최대치 (임시 상수) */
    static inline constexpr int kBroadcastTimeoutSec = 30;
    static inline constexpr int kBroadcastRespMax    = 7;
    static inline constexpr int kValidTimeoutSec     = 10;

    /* ctor */
    MessageService(network::MessageSender&              sender,
                   network::MessageReceiver&            receiver,
                   const persistence::OvmAddressRepository& repo,
                   service::ErrorService&               err,
                   service::InventoryService&           inv,
                   service::PrepaymentService&          prepay,
                   domain::inventory&                   drink);

    /* UC-8  */
    void broadcastStock(const domain::Drink& drink);

    /* UC-16 (송신측) */
    void sendPrePayReq(const domain::Order& order);

    /* UC-17 */
    bool validOVMStock(const std::string& vm_id,
                       const domain::Drink& drink,
                       int qty);

private:
    /* ───────────── internal helpers ───────────── */
    static std::string           myId();
    static std::pair<int,int>    myCoord();

    /* msg handlers (subscribe callback) */
    void handleReqStock (const network::Message& msg);
    void handleRespStock(const network::Message& msg);
    void handleReqPrepay(const network::Message& msg);
    void handleRespPrepay(const network::Message& msg);

    /* 외부에서 직접 호출 안 함 */
    void respondPrepayReq(const domain::Order& order);
    void onMessage(const network::Message&);            // (unused)

    /* ───────────── pending PREPAY 1-shot 관리 ───────────── */
    struct PendingPrepay {
        domain::Order      order;
        std::future<void>  timer_future;   // 30s watchdog
    };
    std::optional<PendingPrepay> pending_;              // option 3 : 1건만

    void startPrepayTimer();
    void cancelPrepayTimer();

    /* ───────────── data members ───────────── */
    network::MessageSender&                sender_;
    network::MessageReceiver&              receiver_;
    const persistence::OvmAddressRepository& repo_;

    service::ErrorService&                 errSvc_;
    service::InventoryService&             invSvc_;
    service::PrepaymentService&            prepaySvc_;

    domain::inventory&                    drink_; // 재고 정보

    /* RESP_STOCK 병렬 수집용 */
    std::vector<network::Message>          resp_cache_;
    std::mutex                             resp_mtx_;
    std::condition_variable                resp_cv_;
};

} // namespace application
