#include "service/MessageService.hpp"

#include "domain/vendingMachine.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <thread>

#include <service/PrepaymentService.hpp>



/*  ───────────── 지역-스코프 전역 (임시) ───────────── */
static domain::VendingMachine vm{"T5", {123, 123}};   // TODO: 1차 때는 그냥 변수 사용(본인 자판기의 정보를 저장하는 레포지토리 문서상 구현 안함)  
using service::MessageService;

/* ��������������������������������������� �젙�쟻 helper ��������������������������������������� */
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

/* ��������������������������������������� UC-8 : 釉뚮줈�뱶罹먯뒪�듃 �넚�떊 ��������������������������������������� */
void MessageService::broadcastStock(const domain::Drink& drink)
{
    network::Message req;
    req.msg_type = network::Message::Type::REQ_STOCK;
    req.src_id   = myId();
    req.dst_id   = "0";               // broadcast湲� �븣臾몄뿉 0�쑝濡� �꽕�젙
    req.msg_content = {
        {"item_code", drink.getCode()},
        {"item_num",  "01"} // 1二쇰Ц 1媛��닔 怨좎젙
    };

    try { sender_.send(req); }
    catch(const std::exception& e){
        errSvc_.logError(std::string("broadcastStock failed: ")+e.what());
    }
}

/* ��������������������������������������� UC-16 : REQ_PREPAY �넚�떊 ��������������������������������������� */
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

        /* pending �벑濡� & watchdog �떆�옉 */
        pending_.emplace(PendingPrepay{order});
        startPrepayTimer();
    }
    catch(const std::exception& e){
        errSvc_.logError(std::string("sendPrePayReq failed: ")+e.what());
    }
}

/* ��������������������������������������� ����씠癒� helpers ��������������������������������������� */
void MessageService::startPrepayTimer() // 紐⑤뱺 �슂泥���� 30珥� �씠�궡�뿉 �쓳�떟�씠 ����빞�븿
{
    if (pending_ && pending_->timer_future.valid())
        cancelPrepayTimer();   // 湲곗〈 ����씠癒� �엳�쑝硫� �젣嫄�


    pending_->timer_future = std::async(std::launch::async, [this]{
        std::this_thread::sleep_for(std::chrono::seconds(30));

        if (pending_) {                 // 30s �씠�썑�뿉�룄 �쓳�떟 X
            errSvc_.logError("PREPAY timeout (30s)"); //�뿉�윭諛쒖깮
            pending_.reset();
        }
    });
}

void MessageService::cancelPrepayTimer()
{
    if (pending_ && pending_->timer_future.valid())
        pending_->timer_future.wait_for(std::chrono::seconds(0));   // 利됱떆 join
}

/* ��������������������������������������� UC-15 : REQ_PREPAY �닔�떊 �썑 �쓳�떟�빖�뱾�윭 ��������������������������������������� */
void MessageService::respondPrepayReq(const domain::Order& order) //
{

        service::PrepaymentService prepaySvc;
        prepaySvc.saveCode(order); //선결제 코드 저장하는 메소드 호출(인증코드 저장)
        
        

    /* 1. 선결제 코드 검증 */
    
    if(!prepaySvc_.isValid(order.certCode())){ //�쓽�룄�뒗 諛쒓툒�맂 肄붾뱶�씤吏� �솗�씤�븯怨� ����옣�븯�뒗 寃껋씠�뿀�쑝�굹 PrepaymentService�뿉�꽌 �씠瑜� �쐞�븳 肄붾뱶媛� �뾾�뼱�꽌 �쑀�슚�꽦 寃��궗留� 吏꾪뻾
        errSvc_.logError("respondPrepayReq: invalid code");
        return;
    }

    /*  3. �옱怨� �솗蹂� �꽦怨�, �떎�뙣�뿉 �뵲�씪�꽌 �쓳�떟 */
    if(invSvc_.getSaleValid(order.drink().getCode())){ // �옱怨좉�� �엳�쓣 �븣 媛��뒫�븳 寃쎌슦 T瑜� 蹂대깂
        network::Message resp;
        resp.msg_type = network::Message::Type::RESP_PREPAY;
        resp.src_id   = myId();
        resp.dst_id   = order.vmId();
        resp.msg_content = {
        {"availability","T"},
        {"item_num",    std::to_string(order.quantity())},
        {"item_code", order.drink().getCode()} //표 형식 따라서 작성  //메시지 만든다
        };

        try { //인증코드 및 음료정보 저장하고, 메시지 보내고, 재고 감소
            service::PrepaymentService prepaySvc;
            prepaySvc.saveCode(order); //선결제 코드 저장하는 메소드 호출(인증코드 저장)
            sender_.send(resp); 
            invSvc_.reduceDrink(order.drink().getCode()); 
        }
        catch(const std::exception& e){
            errSvc_.logError(std::string("RESP_PREPAY send() failed: ")+e.what());
        }
    }else{ //재고가 없을 때 불가능한 경우 메시지 보낸다. 
        network::Message resp;
        resp.msg_type = network::Message::Type::RESP_PREPAY;
        resp.src_id   = myId();
        resp.dst_id   = order.vmId();
        resp.msg_content = {
        {"availability","F"},
        {"item_num",    std::to_string(order.quantity())},
        {"item_code", order.drink().getCode()} //�몴 �삎�떇 �뵲�씪�꽌 �옉�꽦 
    };

        try { sender_.send(resp); }
        catch(const std::exception& e){
            errSvc_.logError(std::string("RESP_PREPAY send() failed: ")+e.what());
        }
    }
   
    
}

/* ��������������������������������������� �빖�뱾�윭�뱾 ��������������������������������������� */


/* ��������������������������������������� 釉뚮줈�뱶罹먯뒪�듃 諛쏆븯�쓣 �븣 �룎�젮二쇰뒗 �빖�뱾�윭  ��������������������������������������� */
void MessageService::handleReqStock(const network::Message& msg)
{
    bool empty = !invSvc_.getSaleValid(msg.msg_content.at("item_code"));

    network::Message resp;
    resp.msg_type = network::Message::Type::RESP_STOCK;
    resp.src_id   = myId();
    resp.dst_id   = msg.src_id; //釉뚮줈�뱶罹먯뒪�듃 諛쏆븯�쓣 �븣 �룎�젮二쇰뒗 �빖�뱾�윭 
    resp.msg_content = {
        {"item_code", msg.msg_content.at("item_code")},
        {"item_num",  empty ? "0" : "1"}, //TODO : 지금 원하는 음료의 재고 잔량을 확인하는 메소드가 없음 비어있는지만 알 수 있음.
    };
    try { sender_.send(resp); }
    catch(const std::exception& e){
        errSvc_.logError(std::string("handleReqStock send() failed: ")+e.what());
    }
}

/* ��������������������������������������� 釉뚮줈�뱶罹먯뒪�듃 �쓳�떟 諛쏆븯�쓣 �븣 紐⑥븘�꽌 distanceService�뿉 二쇰뒗 �빖�뱾�윭  ��������������������������������������� */
void MessageService::handleRespStock(const network::Message& msg)
{
    std::lock_guard lg(resp_mtx_);
    resp_cache_.push_back(msg);
    if(resp_cache_.size() >= kBroadcastRespMax)
        resp_cv_.notify_all();
    
}


//UC 15 핸들러입니다
void MessageService::handleReqPrepay(const network::Message& msg)
{
    /*  msg �넂 Order �뙆�떛 �썑 respondPrepayReq(order) �샇異� */
    try {
        // 메세지 오면 order 객체 생성하고 코드 붙여서 저장 
        domain::Drink d{"", 0, msg.msg_content.at("item_code")};    // 드링크 하나 만든다
        domain::Order order{
            msg.src_id,     // vmId  (蹂대궦 履�)
            d,
            1,
            msg.msg_content.at("cert_code"),
            "Pending"
        };//order만들었음


        //UC15으로 넘어감 
        respondPrepayReq(order);
    }
    catch (const std::exception& e) {
        errSvc_.logError(std::string("handleReqPrepay parse error: ") + e.what());
    }
}

void MessageService::handleRespPrepay(const network::Message& msg)
{
    /* pending_ �씠 �엳�쑝硫� �궡 �슂泥��뿉 ����븳 �쓳�떟�엫�쓣 �븣怨� �씠寃껊쭔 �슂泥� -> �룞�떆�뿉 �븳紐낆쓽 �궗�슜�옄媛� �옄�뙋湲곕�� �궗�슜�븳�떎怨� 媛��젙�븯湲� �븣臾몄뿉 �룞�떆 ���湲곌�� �뾾�쓬(釉뚮줈�뱶罹먯뒪�듃�젣�쇅)*/
    if (!pending_) {
        errSvc_.logError("unexpected RESP_PREPAY (no pending)");
        return;
    }

    cancelPrepayTimer();

    const bool ok = (msg.msg_content.at("availability") == "T");
    if (ok) {
        // TODO: Controller.onPrepayApproved(pending_->order); -> 컨트롤러 상에서 구현 안됨
        std::cout << "[MessageService] PREPAY approved\n";
    } else {
        errSvc_.logError("PREPAY declined (availability == F)");
    }
    pending_.reset();
}

 
