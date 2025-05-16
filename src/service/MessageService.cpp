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
//     // 1) REQ_STOCK �޽��� ����
//     Message req;
//     req.msg_type = Message::Type::REQ_STOCK;
//     req.src_id   = /* �� ���Ǳ� ID, ��: */ "T5";   // TODO: runtime�� ���Ե� �� ID ���
//     req.dst_id   = vm_id;
//     req.msg_content = {
//         {"item_code", /* TODO: üũ�� ���� �ڵ� */ "00"},
//         {"item_num",  /* TODO: üũ�� ����, ���� "01" */    "01"}
//     };

//     // 2) ��û ����
//     sender_.send(req);

//     // 3) ���� ���� ���
//     //    TODO: ���� �񵿱� �ݹ�/condition_variable ���� ����Ͽ�
//     //    receiver_ ���� RESP_STOCK �� �ް�, msg_content["item_num"] > 0 ���� �Ǵ�
//     //
//     //    ����� stub�̹Ƿ� �׻� true ��ȯ
//     return true;
// } �̰� ���� ��Ʈ�ѷ��� ��û�ϴ� ��� ��Ȯ�� ���ǵǸ� ���缭 ���� 