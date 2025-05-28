#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
// --- ��ü ���ǰ� �ʿ��� ��� ���ϵ��� ���� ���� ---
#include "network/message.hpp"         // network::Message �� network::Message::Type ����
#include "network/MessageSender.hpp"   // network::MessageSender ����
#include "network/MessageReceiver.hpp" // network::MessageReceiver �� network::EnumClassHash ����
#include "service/ErrorService.hpp"    // service::ErrorService ����

// --- ���� ���� (���� �ּ�ȭ�ǰų� �ʿ� ���� �� ����) ---
// ������ �̹� ��ü ����� ���������Ƿ�, �Ʒ� ���� ������� ��κ� ���ʿ��մϴ�.
// namespace network {
//     // struct Message; // �̹� ������ message.hpp ����
//     // class MessageSender; // �̹� ������ MessageSender.hpp ����
//     // class MessageReceiver; // �̹� ������ MessageReceiver.hpp ����
// }
// namespace service {
//     // class ErrorService; // �̹� ������ ErrorService.hpp ����
// }


namespace service {

// �ݹ� �Լ� Ÿ�� ���� (���� network::Message�� ������ Ÿ����)
using GenericMessageHandler = std::function<void(const network::Message& message)>;

class MessageService {
public:
    MessageService(
        network::MessageSender& sender,     // OK: MessageSender.hpp ���Ե�
        network::MessageReceiver& receiver, // OK: MessageReceiver.hpp ���Ե�
        service::ErrorService& errorService, // OK: ErrorService.hpp ���Ե�
        const std::string& myVmId,
        int myCoordX,
        int myCoordY
    );

    void startReceivingMessages();

    // --- �޽��� �۽� �޼ҵ� ---
    // network::Message Ÿ���� ���ڳ� ��ȯ������ ����ϴ� �޼ҵ����
    // network/message.hpp�� ���ԵǾ� �����Ƿ� ���� ����.
    void sendStockRequestBroadcast(const std::string& drinkCode);
    void sendPrepaymentReservationRequest(const std::string& targetVmId, const std::string& drinkCode, const std::string& authCode);
    void sendStockResponse(const std::string& destinationVmId, const std::string& drinkCode, int currentStock);
    void sendPrepaymentReservationResponse(const std::string& destinationVmId, const std::string& drinkCode, bool available);

    // --- �޽��� ���� �ڵ鷯 ��� ---
    void registerMessageHandler(network::Message::Type type, GenericMessageHandler handler); // network::Message::Type ���

private:
    network::MessageSender& messageSender_;     // OK
    network::MessageReceiver& messageReceiver_; // OK
    service::ErrorService& errorService_;       // OK

    std::string myVmId_;
    int myCoordX_;
    int myCoordY_;

    void onMessageReceived(const network::Message& msg); // network::Message ���

    // network::Message::Type�� network::EnumClassHash ���
    std::unordered_map<network::Message::Type, GenericMessageHandler, network::EnumClassHash> messageHandlers_;
};

} // namespace service