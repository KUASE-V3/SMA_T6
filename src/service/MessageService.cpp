#include "service/MessageService.hpp"

#include "domain/vendingMachine.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <thread>

/*  �������������������������� ����-������ ���� (�ӽ�) �������������������������� */
static domain::VendingMachine vm{"T5", {123, 123}};   // TODO: 1�� ���� �׳� �ӽ÷� ���(�������丮 ������ ���� ����) -> 2�� �� �����Ѵ�. 

using application::MessageService;

/* �������������������������� ���� helper �������������������������� */
std::string MessageService::myId()            { return vm.getId();      }
std::pair<int,int> MessageService::myCoord()  { return vm.getLocation();}

/* �������������������������� ctor �������������������������� */
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

/* �������������������������� UC-8 : ��ε�ĳ��Ʈ �۽� �������������������������� */
void MessageService::broadcastStock(const domain::Drink& drink)
{
    network::Message req;
    req.msg_type = network::Message::Type::REQ_STOCK;
    req.src_id   = myId();
    req.dst_id   = "0";               // broadcast�� ������ 0���� ����
    req.msg_content = {
        {"item_code", drink.getCode()},
        {"item_num",  "01"} // 1�ֹ� 1���� ����
    };

    try { sender_.send(req); }
    catch(const std::exception& e){
        errSvc_.logError(std::string("broadcastStock failed: ")+e.what());
    }
}

/* �������������������������� UC-16 : REQ_PREPAY �۽� �������������������������� */
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

        /* pending ��� & watchdog ���� */
        pending_.emplace(PendingPrepay{order});
        startPrepayTimer();
    }
    catch(const std::exception& e){
        errSvc_.logError(std::string("sendPrePayReq failed: ")+e.what());
    }
}

/* �������������������������� Ÿ�̸� helpers �������������������������� */
void MessageService::startPrepayTimer() // ��� ��û�� 30�� �̳��� ������ �;���
{
    if (pending_ && pending_->timer_future.valid())
        cancelPrepayTimer();   // ���� Ÿ�̸� ������ ����


    pending_->timer_future = std::async(std::launch::async, [this]{
        std::this_thread::sleep_for(std::chrono::seconds(30));

        if (pending_) {                 // 30s ���Ŀ��� ���� X
            errSvc_.logError("PREPAY timeout (30s)"); //�����߻�
            pending_.reset();
        }
    });
}

void MessageService::cancelPrepayTimer()
{
    if (pending_ && pending_->timer_future.valid())
        pending_->timer_future.wait_for(std::chrono::seconds(0));   // ��� join
}

/* �������������������������� UC-15 : REQ_PREPAY ���� �� �����ڵ鷯 �������������������������� */
void MessageService::respondPrepayReq(const domain::Order& order) //
{
    /* 1. ������ �ڵ� ���� */
    
    if(!prepaySvc_.isValid(order.certCode())){ //�ǵ��� �߱޵� �ڵ����� Ȯ���ϰ� �����ϴ� ���̾����� PrepaymentService���� �̸� ���� �ڵ尡 ��� ��ȿ�� �˻縸 ����
        errSvc_.logError("respondPrepayReq: invalid code");
        return;
    }

    /*  3. ��� Ȯ�� ����, ���п� ���� ���� */
    if(invSvc_.getSaleValid(order.drink().getCode())){ // ����� ���� �� ������ ��� T�� ����
        network::Message resp;
        resp.msg_type = network::Message::Type::RESP_PREPAY;
        resp.src_id   = myId();
        resp.dst_id   = order.vmId();
        resp.msg_content = {
        {"availability","T"},
        {"item_num",    std::to_string(order.quantity())},
        {"item_code", order.drink().getCode()} //ǥ ���� ���� �ۼ� 
            /*  2. ��� Ȯ�� �ٵ� ���� �ڵ常���� �κ��丮 �����ؼ� ��� �����ϰ� �ϴ� ���� inventory�� ���� �ȵ�����(2�� �� �����ؾߵ�) */
        };

        try { sender_.send(resp);
            invSvc_.ReqReduceDrink(order.drink().getCode()); // ��� ���ҿ�
        }
        catch(const std::exception& e){
            errSvc_.logError(std::string("RESP_PREPAY send() failed: ")+e.what());
        }
    }else{ //����� ���� �� �Ұ����� ���
        network::Message resp;
        resp.msg_type = network::Message::Type::RESP_PREPAY;
        resp.src_id   = myId();
        resp.dst_id   = order.vmId();
        resp.msg_content = {
        {"availability","F"},
        {"item_num",    std::to_string(order.quantity())},
        {"item_code", order.drink().getCode()} //ǥ ���� ���� �ۼ� 
    };

        try { sender_.send(resp); }
        catch(const std::exception& e){
            errSvc_.logError(std::string("RESP_PREPAY send() failed: ")+e.what());
        }
    }
   
    
}

/* �������������������������� �ڵ鷯�� �������������������������� */


/* �������������������������� ��ε�ĳ��Ʈ �޾��� �� �����ִ� �ڵ鷯  �������������������������� */
void MessageService::handleReqStock(const network::Message& msg)
{
    bool empty = !invSvc_.getSaleValid(msg.msg_content.at("item_code"));

    network::Message resp;
    resp.msg_type = network::Message::Type::RESP_STOCK;
    resp.src_id   = myId();
    resp.dst_id   = msg.src_id; //��ε�ĳ��Ʈ �޾��� �� �����ִ� �ڵ鷯 
    resp.msg_content = {
        {"item_code", msg.msg_content.at("item_code")},
        {"item_num",  empty ? "0" : "1"}, //TODO : ���� ���ϴ� ������ ��� �ܷ��� Ȯ���ϴ� �޼ҵ尡 ���� �ϴ� 1�� ���� ������� ������ ���� 1�� ����, 2�� �� �����ؾߵ�. 
    };
    try { sender_.send(resp); }
    catch(const std::exception& e){
        errSvc_.logError(std::string("handleReqStock send() failed: ")+e.what());
    }
}

/* �������������������������� ��ε�ĳ��Ʈ ���� �޾��� �� ��Ƽ� distanceService�� �ִ� �ڵ鷯  �������������������������� */
void MessageService::handleRespStock(const network::Message& msg)
{
    std::lock_guard lg(resp_mtx_);
    resp_cache_.push_back(msg);
    if(resp_cache_.size() >= kBroadcastRespMax)
        resp_cv_.notify_all();
}

void MessageService::handleReqPrepay(const network::Message& msg)
{
    /*  msg �� Order �Ľ� �� respondPrepayReq(order) ȣ�� */
    try {
        /* �� network::Message �� domain::Order ���� */
        domain::Drink d{"", 0, msg.msg_content.at("item_code")};    // �̸������� �� �� 0 ã�ƿ� �� ������ �ʿ伺 ���� �ʳ�
        domain::Order order{
            msg.src_id,     // vmId  (���� ��)
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
    /* pending_ �� ������ �� ��û�� ���� �������� �˰� �̰͸� ��û -> ���ÿ� �Ѹ��� ����ڰ� ���Ǳ⸦ ����Ѵٰ� �����ϱ� ������ ���� ��Ⱑ ����(��ε�ĳ��Ʈ����)*/
    if (!pending_) {
        errSvc_.logError("unexpected RESP_PREPAY (no pending)");
        return;
    }

    cancelPrepayTimer();

    const bool ok = (msg.msg_content.at("availability") == "T");
    if (ok) {
        // TODO: Controller.onPrepayApproved(pending_->order); -> ��Ʈ�ѷ� �󿡼� ���� �ȵ�.. 2�� �� ����
        std::cout << "[MessageService] PREPAY approved\n";
    } else {
        errSvc_.logError("PREPAY declined (availability == F)");
    }
    pending_.reset();
}

void MessageService::onMessage(const network::Message&) { /* unused */ } 
        // TODO: �������� ������ ���ó ��� ���� ����, 2�� �� ���� ����
 
