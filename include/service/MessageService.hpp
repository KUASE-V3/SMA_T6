#pragma once
/*───────────────────────────────────────────────────────────────*
 *  MessageService
 *  ───────────────
 *  네트워크 계층( MessageSender / MessageReceiver ) 만을 사용해
 *  VM 간 메시지 송·수신 로직을 중앙집중식으로 제공한다.
 *
 *  ※ 실제 재고·결제·좌표 계산 등은 다른 도메인 서비스에 위임하며,
 *     이 파일에는 구체 구현을 넣지 않는다. (모두 TODO 마커)
 *───────────────────────────────────────────────────────────────*/

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <mutex>
#include <condition_variable>

#include "network/message.hpp"
#include "domain/drink.h"
#include "domain/order.h"


namespace network {
    class MessageSender;
    class MessageReceiver;
}
namespace persistence {
    class OvmAddressRepository;
}

namespace application {

class MessageService {
public:
    /*────────────────── 생성자 ──────────────────*/
    MessageService(network::MessageSender&                 sender,
                   network::MessageReceiver&               receiver,
                   const persistence::OvmAddressRepository& repo);

    /*────────────────── UC-8 ──────────────────*/
    /* 모든 VM 에게 재고 조회(REQ_STOCK) 브로드캐스트 */
    void broadcastStock(const domain::Drink& drink);
    /*────────────────── UC-9 ──────────────────*/
    /* RESP_STOCK 리스트로부터 가장 가까운 VM id 결정 */
    std::optional<std::string>
        getDistance(const std::vector<network::Message>& stockList);

    /*────────────────── UC-15 / 16 ──────────────────*/
    /* (T₁ → T₂) 선결제 요청 */
    void sendPrePayReq(const domain::Order& order);

    /* (T₂) 선결제 요청 수신 시 응답 */
    void respondPrepayReq(const domain::Order& order);

    /*────────────────── UC-17 ──────────────────*/
    /* 단일 VM 에 재고가 있는지 확인 (동기) */
    bool validOVMStock(const std::string&   vm_id,
                       const domain::Drink& drink,
                       int                  qty = 1);

    /*────────────────── 수신 엔트리 ──────────────────*/
    void onMessage(const network::Message& msg);   // (현재는 subscribe 로 분기)

private:
    /* 네트워크 의존 */
    network::MessageSender&                   sender_;
    network::MessageReceiver&                 receiver_;
    const persistence::OvmAddressRepository&  repo_;

    /* 브로드캐스트 응답 캐시 */
    std::mutex                                resp_mtx_;
    std::vector<network::Message>             resp_cache_;
    std::condition_variable                   resp_cv_;      // → 필요 시 사용

    /* 내부 헬퍼 */
    void handleReqStock (const network::Message& msg);
    void handleRespStock(const network::Message& msg);
    void handleReqPrepay(const network::Message& msg);
    void handleRespPrepay(const network::Message& msg);
};

} // namespace application
