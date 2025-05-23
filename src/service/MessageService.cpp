#include "service/MessageService.hpp"

#include "domain/vendingMachine.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <thread>

/*  ───────────── 지역-스코프 전역 (임시) ───────────── */
static domain::VendingMachine vm{"T5", {123, 123}};   // TODO: 1차 때는 그냥 임시로 사용(레포지토리 문서상 구현 안함) -> 2차 때 참조한다. 

using application::MessageService;

/* ───────────── 정적 helper ───────────── */
std::string MessageService::myId()            { return vm.getId();      }
std::pair<int,int> MessageService::myCoord()  { return vm.getLocation();}

/* ───────────── ctor ───────────── */
MessageService::MessageService(network::MessageSender&              sender,
                               network::MessageReceiver&            receiver,
                               const persistence::OvmAddressRepository& repo,
                               service::ErrorService&               err,
                               service::InventoryService&           inv,
                               service::PrepaymentService&          prepay,
                               domain::inventory&                   drink) 
    : sender_(sender)
    , receiver_(receiver)
    , repo_(repo)
    , errSvc_(err)
    , invSvc_(inv)
    , prepaySvc_(prepay)
    , drink_(drink)

{
    using T = network::Message::Type;
    receiver_.subscribe(T::REQ_STOCK,   [this](auto&m){ handleReqStock(m);   });
    receiver_.subscribe(T::RESP_STOCK,  [this](auto&m){ handleRespStock(m);  });
    receiver_.subscribe(T::REQ_PREPAY,  [this](auto&m){ handleReqPrepay(m);  });
    receiver_.subscribe(T::RESP_PREPAY, [this](auto&m){ handleRespPrepay(m); });
}

/* ───────────── UC-8 : 브로드캐스트 송신 ───────────── */
void MessageService::broadcastStock(const domain::Drink& drink)
{
    network::Message req;
    req.msg_type = network::Message::Type::REQ_STOCK;
    req.src_id   = myId();
    req.dst_id   = "0";               // broadcast기 때문에 0으로 설정
    req.msg_content = {
        {"item_code", drink.getCode()},
        {"item_num",  "01"} // 1주문 1갯수 고정
    };

    try { sender_.send(req); }
    catch(const std::exception& e){
        errSvc_.logError(std::string("broadcastStock failed: ")+e.what());
    }
}

/* ───────────── UC-16 : REQ_PREPAY 송신 ───────────── */
void MessageService::sendPrePayReq(const domain::Order& order)
{
    network::Message msg;
    msg.msg_type = network::Message::Type::REQ_PREPAY;
    msg.src_id   = myId();
    msg.dst_id   = order.vmId();
    msg.msg_content = {
        {"item_code", order.drink().getCode()},
        {"item_num",  std::to_string(order.quantity())}, 
        {"cert_code", order.certCode()}
    };

    try {
        sender_.send(msg);

        /* pending 등록 & watchdog 시작 */
        pending_.emplace(PendingPrepay{order});
        startPrepayTimer();
    }
    catch(const std::exception& e){
        errSvc_.logError(std::string("sendPrePayReq failed: ")+e.what());
    }
}

/* ───────────── 타이머 helpers ───────────── */
void MessageService::startPrepayTimer() // 모든 요청은 30초 이내에 응답이 와야함
{
    if (pending_ && pending_->timer_future.valid())
        cancelPrepayTimer();   // 기존 타이머 있으면 제거


    pending_->timer_future = std::async(std::launch::async, [this]{
        std::this_thread::sleep_for(std::chrono::seconds(30));

        if (pending_) {                 // 30s 이후에도 응답 X
            errSvc_.logError("PREPAY timeout (30s)"); //에러발생
            pending_.reset();
        }
    });
}

void MessageService::cancelPrepayTimer()
{
    if (pending_ && pending_->timer_future.valid())
        pending_->timer_future.wait_for(std::chrono::seconds(0));   // 즉시 join
}

/* ───────────── UC-15 : REQ_PREPAY 수신 후 응답핸들러 ───────────── */
void MessageService::respondPrepayReq(const domain::Order& order) //
{
    /* 1. 선결제 코드 검증 */
    
    if(!prepaySvc_.isValid(order.certCode())){ //의도는 발급된 코드인지 확인하고 저장하는 것이었으나 PrepaymentService에서 이를 위한 코드가 없어서 유효성 검사만 진행
        errSvc_.logError("respondPrepayReq: invalid code");
        return;
    }

    /*  3. 재고 확보 성공, 실패에 따라서 응답 */
    if(invSvc_.getSaleValid(order.drink().getCode())){ // 재고가 있을 때 가능한 경우 T를 보냄
        network::Message resp;
        resp.msg_type = network::Message::Type::RESP_PREPAY;
        resp.src_id   = myId();
        resp.dst_id   = order.vmId();
        resp.msg_content = {
        {"availability","T"},
        {"item_num",    std::to_string(order.quantity())},
        {"item_code", order.drink().getCode()} //표 형식 따라서 작성 
            /*  2. 재고 확보 근데 음료 코드만으로 인벤토리 접근해서 재고 감소하게 하는 것이 inventory에 구현 안돼있음(2차 때 진행해야됨) */
        };

        try { sender_.send(resp);
            invSvc_.reduceDrink(order.drink().getCode()); // 재고 감소와
        }
        catch(const std::exception& e){
            errSvc_.logError(std::string("RESP_PREPAY send() failed: ")+e.what());
        }
    }else{ //재고가 없을 때 불가능한 경우
        network::Message resp;
        resp.msg_type = network::Message::Type::RESP_PREPAY;
        resp.src_id   = myId();
        resp.dst_id   = order.vmId();
        resp.msg_content = {
        {"availability","F"},
        {"item_num",    std::to_string(order.quantity())},
        {"item_code", order.drink().getCode()} //표 형식 따라서 작성 
    };

        try { sender_.send(resp); }
        catch(const std::exception& e){
            errSvc_.logError(std::string("RESP_PREPAY send() failed: ")+e.what());
        }
    }
   
    
}

/* ───────────── 핸들러들 ───────────── */


/* ───────────── 브로드캐스트 받았을 때 돌려주는 핸들러  ───────────── */
void MessageService::handleReqStock(const network::Message& msg)
{
    bool empty = !invSvc_.getSaleValid(msg.msg_content.at("item_code"));

    network::Message resp;
    resp.msg_type = network::Message::Type::RESP_STOCK;
    resp.src_id   = myId();
    resp.dst_id   = msg.src_id; //브로드캐스트 받았을 때 돌려주는 핸들러 
    resp.msg_content = {
        {"item_code", msg.msg_content.at("item_code")},
        {"item_num",  empty ? "0" : "1"}, //TODO : 지금 원하는 음료의 재고 잔량을 확인하는 메소드가 없음 일단 1차 때는 비어있지 않으면 응답 1로 고정, 2차 때 구현해야됨. 
    };
    try { sender_.send(resp); }
    catch(const std::exception& e){
        errSvc_.logError(std::string("handleReqStock send() failed: ")+e.what());
    }
}

/* ───────────── 브로드캐스트 응답 받았을 때 모아서 distanceService에 주는 핸들러  ───────────── */
void MessageService::handleRespStock(const network::Message& msg)
{
    std::lock_guard lg(resp_mtx_);
    resp_cache_.push_back(msg);
    if(resp_cache_.size() >= kBroadcastRespMax)
        resp_cv_.notify_all();
}

void MessageService::handleReqPrepay(const network::Message& msg)
{
    /*  msg → Order 파싱 후 respondPrepayReq(order) 호출 */
    try {
        /* ① network::Message → domain::Order 매핑 */
        domain::Drink d{"", 0, msg.msg_content.at("item_code")};    // 이름·가격 모름 → 0 찾아올 수 있으나 필요성 없지 않나
        domain::Order order{
            msg.src_id,     // vmId  (보낸 쪽)
            d,
            std::stoi(msg.msg_content.at("item_num")),
            msg.msg_content.at("cert_code"),
            "Pending"
        };

        
        respondPrepayReq(order);
    }
    catch (const std::exception& e) {
        errSvc_.logError(std::string("handleReqPrepay parse error: ") + e.what());
    }
}

void MessageService::handleRespPrepay(const network::Message& msg)
{
    /* pending_ 이 있으면 내 요청에 대한 응답임을 알고 이것만 요청 -> 동시에 한명의 사용자가 자판기를 사용한다고 가정하기 때문에 동시 대기가 없음(브로드캐스트제외)*/
    if (!pending_) {
        errSvc_.logError("unexpected RESP_PREPAY (no pending)");
        return;
    }

    cancelPrepayTimer();

    const bool ok = (msg.msg_content.at("availability") == "T");
    if (ok) {
        // TODO: Controller.onPrepayApproved(pending_->order); -> 컨트롤러 상에서 구현 안됨.. 2차 때 진행
        std::cout << "[MessageService] PREPAY approved\n";
    } else {
        errSvc_.logError("PREPAY declined (availability == F)");
    }
    pending_.reset();
}

void MessageService::onMessage(const network::Message&) { /* unused */ } 
        // TODO: 문서에는 있으나 사용처 없어서 구현 안함, 2차 때 수정 예정
 
