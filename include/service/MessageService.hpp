#include "network/MessageSender.hpp"
#include "network/MessageReceiver.hpp"
#include "network/message.hpp"
#include <functional>

using network::MessageSender;
using network::MessageReceiver;

class MessageService {
public:
    using StockHandler = std::function<void(const Message&)>;
    using PrepayHandler = std::function<void(const Message&)>;

    MessageService(MessageSender& sender, MessageReceiver& receiver);
    void requestStock(const Message& req);
    void onStockResponse(StockHandler cb);
    void requestPrepay(const Message& req);
    void onPrepayResponse(PrepayHandler cb);
    bool validOVMStock(const std::string& vm_id);

private:
    MessageSender& sender_;
    MessageReceiver& receiver_;
    StockHandler stock_cb_;
    PrepayHandler prepay_cb_;
};
