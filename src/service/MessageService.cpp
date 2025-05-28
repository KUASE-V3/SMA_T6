#include "service/MessageService.hpp"
// MessageService.hpp 에서 이미 필요한 헤더들을 include 하고 있음 (MessageSender, MessageReceiver, Message, ErrorService 등)

#include <functional> // std::bind 등 사용 가능성
#include <iostream>   // 디버깅용


namespace service {

MessageService::MessageService(
    network::MessageSender& sender,
    network::MessageReceiver& receiver,
    service::ErrorService& errorService,
    const std::string& myVmId,
    int myCoordX,
    int myCoordY
) : messageSender_(sender),
    messageReceiver_(receiver),
    errorService_(errorService),
    myVmId_(myVmId),
    myCoordX_(myCoordX),
    myCoordY_(myCoordY) {
    // 생성자에서는 핸들러 등록이나 수신 시작을 하지 않음.
    // startReceivingMessages() 메소드가 그 역할을 담당.
}

void MessageService::startReceivingMessages() {
    // Message.hpp에 정의된 모든 Message::Type에 대해
    // onMessageReceived를 호출하는 람다를 MessageReceiver에 등록합니다.
    messageReceiver_.subscribe(network::Message::Type::REQ_STOCK,
        [this](const network::Message& msg){ this->onMessageReceived(msg); });
    messageReceiver_.subscribe(network::Message::Type::RESP_STOCK,
        [this](const network::Message& msg){ this->onMessageReceived(msg); });
    messageReceiver_.subscribe(network::Message::Type::REQ_PREPAY,
        [this](const network::Message& msg){ this->onMessageReceived(msg); });
    messageReceiver_.subscribe(network::Message::Type::RESP_PREPAY,
        [this](const network::Message& msg){ this->onMessageReceived(msg); });

    // MessageReceiver의 수신 시작 메소드 호출
    try {
        messageReceiver_.start();
    } catch (const std::exception& e) {
        // ErrorService.hpp 에 정의된 ErrorType 사용
        errorService_.processOccurredError(ErrorType::NETWORK_COMMUNICATION_ERROR, "메시지 수신 시작 실패: " + std::string(e.what()));
    }
}

// --- 메시지 송신 메소드 ---

void MessageService::sendStockRequestBroadcast(const std::string& drinkCode) {
    network::Message msg;
    msg.msg_type = network::Message::Type::REQ_STOCK;
    msg.src_id = myVmId_;
    msg.dst_id = "0"; // 브로드캐스트
    msg.msg_content["item_code"] = drinkCode;
    msg.msg_content["item_num"] = "1"; // 기능 요구사항 PFR - 표1 참조

    try {
        messageSender_.send(msg);
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::NETWORK_COMMUNICATION_ERROR, "재고 조회 브로드캐스트 실패: " + std::string(e.what()));
    }
}

void MessageService::sendPrepaymentReservationRequest(const std::string& targetVmId, const std::string& drinkCode, const std::string& authCode) {
    network::Message msg;
    msg.msg_type = network::Message::Type::REQ_PREPAY;
    msg.src_id = myVmId_;
    msg.dst_id = targetVmId;
    msg.msg_content["item_code"] = drinkCode;
    msg.msg_content["item_num"] = "1"; // 기능 요구사항 PFR - 표3 참조 (선결제 요청 시 수량은 보통 1)
    msg.msg_content["cert_code"] = authCode;

    try {
        messageSender_.send(msg);
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::NETWORK_COMMUNICATION_ERROR, "선결제 예약 요청 실패 (" + targetVmId + "): " + std::string(e.what()));
    }
}

void MessageService::sendStockResponse(const std::string& destinationVmId, const std::string& drinkCode, int currentStock) {
    network::Message msg;
    msg.msg_type = network::Message::Type::RESP_STOCK;
    msg.src_id = myVmId_;
    msg.dst_id = destinationVmId;
    msg.msg_content["item_code"] = drinkCode;
    msg.msg_content["item_num"] = std::to_string(currentStock);
    msg.msg_content["coor_x"] = std::to_string(myCoordX_);
    msg.msg_content["coor_y"] = std::to_string(myCoordY_);
    // 기능 요구사항 PFR - 표2 참조

    try {
        messageSender_.send(msg);
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::NETWORK_COMMUNICATION_ERROR, "재고 응답 전송 실패 (" + destinationVmId + "): " + std::string(e.what()));
    }
}

void MessageService::sendPrepaymentReservationResponse(const std::string& destinationVmId, const std::string& drinkCode, bool available) {
    network::Message msg;
    msg.msg_type = network::Message::Type::RESP_PREPAY;
    msg.src_id = myVmId_;
    msg.dst_id = destinationVmId;
    msg.msg_content["item_code"] = drinkCode;
    msg.msg_content["item_num"] = available ? "1" : "0"; // 기능 요구사항 PFR - 표4 참조
    msg.msg_content["availability"] = available ? "T" : "F";

    try {
        messageSender_.send(msg);
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::NETWORK_COMMUNICATION_ERROR, "선결제 예약 응답 전송 실패 (" + destinationVmId + "): " + std::string(e.what()));
    }
}

void MessageService::registerMessageHandler(network::Message::Type type, GenericMessageHandler handler) {
    messageHandlers_[type] = std::move(handler);
}

void MessageService::onMessageReceived(const network::Message& msg) {
    auto it = messageHandlers_.find(msg.msg_type);
    if (it != messageHandlers_.end()) {
        it->second(msg); // 등록된 핸들러 호출
    } else {
        std::cerr << "[MessageService] Warning: No handler registered for message type "
                  << static_cast<int>(msg.msg_type) << " from " << msg.src_id << std::endl;
        // 이 경우 ErrorService를 통해 UNEXPECTED_SYSTEM_ERROR 또는 특정 오류 타입을 보고할 수 있습니다.
        // errorService_.processOccurredError(ErrorType::INVALID_MESSAGE_FORMAT, "처리 핸들러가 없는 메시지 타입 수신: " + std::to_string(static_cast<int>(msg.msg_type)));
    }
}

} // namespace service