#include "service/MessageService.hpp"
// MessageService.hpp에서 이미 필요한 헤더들을 include 하고 있음
// (MessageSender, MessageReceiver, Message, EnumClassHash, ErrorService 등)

#include <functional> // GenericMessageHandler 사용 (이미 hpp에 포함)
#include <string>     // std::to_string 등 사용
// #include <iostream> // 최종 버전에서는 디버깅용 std::cout 제거

namespace service {

/**
 * @brief MessageService 생성자.
 * 의존성 주입을 통해 필요한 객체들을 초기화합니다.
 */
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
}


void MessageService::startReceivingMessages() {
    // Message.hpp에 정의된 모든 Message::Type에 대해
    // onMessageReceived를 호출하는 람다를 MessageReceiver에 등록합니다.
    // 이 람다는 수신된 메시지를 onMessageReceived로 전달하는 역할만 합니다.
    try {
        messageReceiver_.subscribe(network::Message::Type::REQ_STOCK,
            [this](const network::Message& msg){ this->onMessageReceived(msg); });
        messageReceiver_.subscribe(network::Message::Type::RESP_STOCK,
            [this](const network::Message& msg){ this->onMessageReceived(msg); });
        messageReceiver_.subscribe(network::Message::Type::REQ_PREPAY,
            [this](const network::Message& msg){ this->onMessageReceived(msg); });
        messageReceiver_.subscribe(network::Message::Type::RESP_PREPAY,
            [this](const network::Message& msg){ this->onMessageReceived(msg); });

        messageReceiver_.start(); // MessageReceiver의 실제 메시지 수신 루프 시작
    } catch (const std::exception& e) {
        // 네트워크 수신 시작 단계에서 오류 발생 시 (예: 포트 바인딩 실패)
        errorService_.processOccurredError(
            ErrorType::NETWORK_COMMUNICATION_ERROR, // 또는 INITIALIZATION_FAILED
            "메시지 수신 시스템 시작 실패: " + std::string(e.what())
        );
    }
}


// UC8: 재고 조회 브로드캐스트 (PFR 표1)
void MessageService::sendStockRequestBroadcast(const std::string& drinkCode) {
    network::Message msg;
    msg.msg_type = network::Message::Type::REQ_STOCK;
    msg.src_id = myVmId_;
    msg.dst_id = "0"; // "0"은 브로드캐스트를 의미 (
    msg.msg_content["item_code"] = drinkCode;
    msg.msg_content["item_num"] = "1"; // 재고 조회는 항상 1개 음료에 대해 요청

    try {
        messageSender_.send(msg);
    } catch (const std::exception& e) { // MessageSender::send 내부에서 예외 발생 시
        errorService_.processOccurredError(ErrorType::MESSAGE_SEND_FAILED, "재고 조회 브로드캐스트 전송 실패: " + std::string(e.what()));
    }
}

// UC16: 선결제 재고 확보 요청 (PFR 표3)
void MessageService::sendPrepaymentReservationRequest(const std::string& targetVmId, const std::string& drinkCode, const std::string& authCode) {
    network::Message msg;
    msg.msg_type = network::Message::Type::REQ_PREPAY;
    msg.src_id = myVmId_;
    msg.dst_id = targetVmId;
    msg.msg_content["item_code"] = drinkCode;
    msg.msg_content["item_num"] = "1"; // 우리 시스템은 1주문 1음료 원칙이므로 항상 1개 요청
    msg.msg_content["cert_code"] = authCode;

    try {
        messageSender_.send(msg);
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::MESSAGE_SEND_FAILED, "선결제 예약 요청 전송 실패 (" + targetVmId + "): " + std::string(e.what()));
    }
}

// UC17: 재고 조회 요청에 대한 응답 (PFR 표2)
void MessageService::sendStockResponse(const std::string& destinationVmId, const std::string& drinkCode, int currentStock) {
    network::Message msg;
    msg.msg_type = network::Message::Type::RESP_STOCK;
    msg.src_id = myVmId_;
    msg.dst_id = destinationVmId;
    msg.msg_content["item_code"] = drinkCode;
    msg.msg_content["item_num"] = std::to_string(currentStock); // 실제 재고량
    msg.msg_content["coor_x"] = std::to_string(myCoordX_);
    msg.msg_content["coor_y"] = std::to_string(myCoordY_);

    try {
        messageSender_.send(msg);
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::MESSAGE_SEND_FAILED, "재고 응답 전송 실패 (" + destinationVmId + "): " + std::string(e.what()));
    }
}

// UC15: 선결제 재고 확보 요청에 대한 응답 (PFR 표4)
void MessageService::sendPrepaymentReservationResponse(const std::string& destinationVmId, const std::string& drinkCode, int reservedItemNum, bool available) {
    network::Message msg;
    msg.msg_type = network::Message::Type::RESP_PREPAY;
    msg.src_id = myVmId_;
    msg.dst_id = destinationVmId;
    msg.msg_content["item_code"] = drinkCode;
    msg.msg_content["item_num"] = std::to_string(reservedItemNum); // 확보된 (또는 요청받은) 수량
    msg.msg_content["availability"] = available ? "T" : "F"; // 성공 여부

    try {
        messageSender_.send(msg);
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::MESSAGE_SEND_FAILED, "선결제 예약 응답 전송 실패 (" + destinationVmId + "): " + std::string(e.what()));
    }
}

void MessageService::registerMessageHandler(network::Message::Type type, GenericMessageHandler handler) {
    messageHandlers_[type] = std::move(handler);
}


void MessageService::onMessageReceived(const network::Message& msg) {
    auto it = messageHandlers_.find(msg.msg_type);
    if (it != messageHandlers_.end()) {
        it->second(msg); // 등록된 핸들러(UserProcessController의 onXXXReceived) 호출
    } else {
        errorService_.processOccurredError(
            ErrorType::INVALID_MESSAGE_FORMAT, 
            "알 수 없는 메시지 타입 수신: " + std::to_string(static_cast<int>(msg.msg_type)) + " (from " + msg.src_id + ")"
        );
    }
}

} // namespace service
