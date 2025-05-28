#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
// --- 전체 정의가 필요한 헤더 파일들을 먼저 포함 ---
#include "network/message.hpp"         // network::Message 및 network::Message::Type 정의
#include "network/MessageSender.hpp"   // network::MessageSender 정의
#include "network/MessageReceiver.hpp" // network::MessageReceiver 및 network::EnumClassHash 정의
#include "service/ErrorService.hpp"    // service::ErrorService 정의

// --- 전방 선언 (이제 최소화되거나 필요 없을 수 있음) ---
// 위에서 이미 전체 헤더를 포함했으므로, 아래 전방 선언들은 대부분 불필요합니다.
// namespace network {
//     // struct Message; // 이미 위에서 message.hpp 포함
//     // class MessageSender; // 이미 위에서 MessageSender.hpp 포함
//     // class MessageReceiver; // 이미 위에서 MessageReceiver.hpp 포함
// }
// namespace service {
//     // class ErrorService; // 이미 위에서 ErrorService.hpp 포함
// }


namespace service {

// 콜백 함수 타입 정의 (이제 network::Message가 완전한 타입임)
using GenericMessageHandler = std::function<void(const network::Message& message)>;

class MessageService {
public:
    MessageService(
        network::MessageSender& sender,     // OK: MessageSender.hpp 포함됨
        network::MessageReceiver& receiver, // OK: MessageReceiver.hpp 포함됨
        service::ErrorService& errorService, // OK: ErrorService.hpp 포함됨
        const std::string& myVmId,
        int myCoordX,
        int myCoordY
    );

    void startReceivingMessages();

    // --- 메시지 송신 메소드 ---
    // network::Message 타입을 인자나 반환값으로 사용하는 메소드들은
    // network/message.hpp가 포함되어 있으므로 문제 없음.
    void sendStockRequestBroadcast(const std::string& drinkCode);
    void sendPrepaymentReservationRequest(const std::string& targetVmId, const std::string& drinkCode, const std::string& authCode);
    void sendStockResponse(const std::string& destinationVmId, const std::string& drinkCode, int currentStock);
    void sendPrepaymentReservationResponse(const std::string& destinationVmId, const std::string& drinkCode, bool available);

    // --- 메시지 수신 핸들러 등록 ---
    void registerMessageHandler(network::Message::Type type, GenericMessageHandler handler); // network::Message::Type 사용

private:
    network::MessageSender& messageSender_;     // OK
    network::MessageReceiver& messageReceiver_; // OK
    service::ErrorService& errorService_;       // OK

    std::string myVmId_;
    int myCoordX_;
    int myCoordY_;

    void onMessageReceived(const network::Message& msg); // network::Message 사용

    // network::Message::Type과 network::EnumClassHash 사용
    std::unordered_map<network::Message::Type, GenericMessageHandler, network::EnumClassHash> messageHandlers_;
};

} // namespace service