#pragma once
#include <string>
#include "domain/Drink.hpp"
#include "domain/Order.hpp"
#include "network/message.hpp"
#include <vector>         

namespace network { class MessageSender; class MessageReceiver; }
namespace persistence { class OvmAddressRepository; }

namespace application {

/**
 * MessageService
 * ?- MessageSender / MessageReceiver 를 통해
 * ?  메시지 송수신을 담당하는 서비스.
 *
 * ?? ErrorService 연동은 TODO: 로 남겨둠.
 */
class MessageService {
public:
    /**
     * ctor
     * @param sender   MessageSender (Network Layer)
     * @param receiver MessageReceiver (Network Layer)
     * @param repo     OvmAddressRepository ? VM ID ↔ endpoint 조회용
     */
    MessageService(network::MessageSender&   sender,
                   network::MessageReceiver& receiver,
                   const persistence::OvmAddressRepository& repo);

    /* ─────────────────── UC-8 ─────────────────── */

    /**
     * broadcastStock(drink)
     * ?현재 자판기의 재고 현황(Drink) 조회 요청을 모든 VM에 브로드캐스트한다.
     * ?→ 성공 시 각 VM이 RESP_STOCK 반환
     */
    void broadcastStock(const domain::Drink& drink);

    /* ─────────────────── UC-9 ─────────────────── */

    /**
     * getDistance(list)
     * ?(다이어그램에서는 DistanceService 방향 호출이지만)
     * ?브로드캐스트 응답 리스트를 받아 가장 가까운 VM을 계산하는 보조 함수.
     */
     void getDistance(const std::vector<network::Message>& stockList);

    /* ─────────────────── UC-15 / UC-16 ─────────────────── */

    /**
     * sendPrePayReq(order)
     * ?T1 → T2 단일 VM에게 선결제 요청(REQ_PREPAY) 전송
     */
    void sendPrePayReq(const domain::Order& order);

    /**
     * respondPrepayReq(order)
     * ?수신 측(T2)에서 REQ_PREPAY 도착 시 호출.
     * ?재고 확보 & PrePaymentCode 생성 후 RESP_PREPAY 로 응답한다.
     */
    void respondPrepayReq(const domain::Order& order);

    /* ─────────────────── UC-17 ─────────────────── */

    /**
     * validOVMStock(vm_id, drink, qty)
     * ?특정 VM(id)에게 재고가 있는지 확인.
     * ?REQ_STOCK / RESP_STOCK 왕복 후 Bool 반환.
     */
    bool validOVMStock(const std::string&   vm_id,
                       const domain::Drink& drink,
                       int                  qty = 1);

    /* ─────────────────── 수신 엔트리 포인트 ─────────────────── */

    /**
     * onMessage(msg)
     * ?MessageReceiver 가 수신한 모든 메시지는
     * ?MessageService::onMessage 로 집결 → 유스케이스별 분기 처리.
     */
    void onMessage(const network::Message& msg);

private:
    /* 의존 객체 */
    network::MessageSender&                 sender_;
    network::MessageReceiver&               receiver_;
    const persistence::OvmAddressRepository& repo_;

    /* 내부 헬퍼 */
    void handleReqStock (const network::Message& msg);
    void handleRespStock(const network::Message& msg);
    void handleReqPrepay(const network::Message& msg);
    void handleRespPrepay(const network::Message& msg);
};

} // namespace application
