#include "service/MessageService.hpp"

MessageService::MessageService(MessageSender& sender, MessageReceiver& receiver)
  : sender_(sender), receiver_(receiver) {
    receiver_.subscribe(Message::Type::RESP_STOCK, [this](const Message& m){
        if (stock_cb_) stock_cb_(m);
    });
    receiver_.subscribe(Message::Type::RESP_PREPAY, [this](const Message& m){
        if (prepay_cb_) prepay_cb_(m);
    });
}

void MessageService::requestStock(const Message& req) { sender_.send(req); }
void MessageService::onStockResponse(StockHandler cb) { stock_cb_ = std::move(cb); }
void MessageService::requestPrepay(const Message& req) { sender_.send(req); }
void MessageService::onPrepayResponse(PrepayHandler cb) { prepay_cb_ = std::move(cb); }
// bool MessageService::validOVMStock(const std::string& vm_id) {
//     // 1) REQ_STOCK 메시지 구성
//     Message req;
//     req.msg_type = Message::Type::REQ_STOCK;
//     req.src_id   = /* 내 자판기 ID, 예: */ "T5";   // TODO: runtime에 주입된 내 ID 사용
//     req.dst_id   = vm_id;
//     req.msg_content = {
//         {"item_code", /* TODO: 체크할 음료 코드 */ "00"},
//         {"item_num",  /* TODO: 체크할 수량, 보통 "01" */    "01"}
//     };

//     // 2) 요청 전송
//     sender_.send(req);

//     // 3) 응답 수신 대기
//     //    TODO: 실제 비동기 콜백/condition_variable 등을 사용하여
//     //    receiver_ 에서 RESP_STOCK 을 받고, msg_content["item_num"] > 0 여부 판단
//     //
//     //    현재는 stub이므로 항상 true 반환
//     return true;
// } 이거 추후 컨트롤러가 요청하는 방식 정확히 정의되면 맞춰서 ㅅ정 