#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map> // std::map���� �����

// --- �ʿ��� Ÿ�Ե��� ��ü ���Ǹ� ���� ���� ---
#include "network/message.hpp"         // network::Message �� network::Message::Type ����
#include "network/MessageSender.hpp"   // network::MessageSender ����
#include "network/MessageReceiver.hpp" // network::MessageReceiver �� network::EnumClassHash ����
#include "service/ErrorService.hpp"    // service::ErrorService ����

namespace service {

// �ݹ� �Լ� Ÿ�� ����: ���ŵ� ��Ʈ��ũ �޽����� ó���ϱ� ���� �Ϲ� �ڵ鷯
using GenericMessageHandler = std::function<void(const network::Message& message)>;

/**
 * @brief �ٸ� ���Ǳ���� �޽��� �ۼ����� ����ϴ� �����Դϴ�.
 * MessageSender�� MessageReceiver�� ����Ͽ� ��Ʈ��ũ ����� �����ϰ�,
 * ���ŵ� �޽����� ���� ��ϵ� �ڵ鷯(�ַ� UserProcessController�� �޼ҵ�)�� ȣ���մϴ�.
 */
class MessageService {
public:
    /**
     * @brief MessageService ������.
     * @param sender �޽��� �۽��� ���� MessageSender ��ü�� ���� ����.
     * @param receiver �޽��� ������ ���� MessageReceiver ��ü�� ���� ����.
     * @param errorService ���� ó���� ���� ErrorService ��ü�� ���� ����.
     * @param myVmId ���� ���Ǳ��� ���� ID.
     * @param myCoordX ���� ���Ǳ��� X ��ǥ (RESP_STOCK �޽��� ���� �� ���).
     * @param myCoordY ���� ���Ǳ��� Y ��ǥ (RESP_STOCK �޽��� ���� �� ���).
     */
    MessageService(
        network::MessageSender& sender,
        network::MessageReceiver& receiver,
        service::ErrorService& errorService,
        const std::string& myVmId,
        int myCoordX,
        int myCoordY
    );

    /**
     * @brief MessageReceiver�� ���� �޽��� ������ �����ϵ��� �����մϴ�.
     * ���������� �� �޽��� Ÿ�Կ� ���� onMessageReceived�� ȣ���ϴ� �ڵ鷯�� ����ϰ�,
     * MessageReceiver�� start()�� ȣ���մϴ�.
     */
    void startReceivingMessages();

    // --- �޽��� �۽� �޼ҵ� ---

    /**
     * @brief ��� ��ȸ ��û (REQ_STOCK)�� ��� �ٸ� ���Ǳ⿡ ��ε�ĳ��Ʈ�մϴ�. (UC8)
     * PFR ǥ1: ��� Ȯ�� ��û ���� msg format �ؼ�.
     * @param drinkCode ��ȸ�� ������ �ڵ�.
     */
    void sendStockRequestBroadcast(const std::string& drinkCode);

    /**
     * @brief Ư�� ���Ǳ⿡ ������ ��� Ȯ�� ��û (REQ_PREPAY)�� �����մϴ�. (UC16)
     * PFR ǥ3: ������ ��û ���� msg format �ؼ�.
     * @param targetVmId ��� ���Ǳ��� ID.
     * @param drinkCode ������ ������ �ڵ�.
     * @param authCode ������ ���� �ڵ�.
     */
    void sendPrepaymentReservationRequest(const std::string& targetVmId, const std::string& drinkCode, const std::string& authCode);

    /**
     * @brief �ٸ� ���Ǳ��� ��� ��ȸ ��û(REQ_STOCK)�� ���� ����(RESP_STOCK)�� �����մϴ�. (UC17)
     * PFR ǥ2: ��� Ȯ�� ���� ���� msg format �ؼ�.
     * @param destinationVmId ������ ���� ���Ǳ�(��û�� ���� ���Ǳ�)�� ID.
     * @param drinkCode ��ȸ ��û���� ������ �ڵ�.
     * @param currentStock �ش� ������ ���� ��� (0~99).
     */
    void sendStockResponse(const std::string& destinationVmId, const std::string& drinkCode, int currentStock);

    /**
     * @brief �ٸ� ���Ǳ��� ������ ��� Ȯ�� ��û(REQ_PREPAY)�� ���� ����(RESP_PREPAY)�� �����մϴ�. (UC15)
     * PFR ǥ4: ������ ���� ���� ���� ���� msg format �ؼ�.
     * @param destinationVmId ������ ���� ���Ǳ�(��û�� ���� ���Ǳ�)�� ID.
     * @param drinkCode ��û���� ������ �ڵ�.
     * @param reservedItemNum ������ Ȯ��(����)�� ������ ���� (���� �� ��û ����, ���� �� 0).
     * @param available ��� Ȯ�� �� ������ ó�� ���� ���� (T/F).
     */
    void sendPrepaymentReservationResponse(const std::string& destinationVmId, const std::string& drinkCode, int reservedItemNum, bool available);

    // --- �޽��� ���� �ڵ鷯 ��� ---

    /**
     * @brief Ư�� �޽��� Ÿ�Կ� ���� ó�� �ڵ鷯�� ����մϴ�.
     * UserProcessController �� �ܺο��� �� �޼ҵ带 ȣ���Ͽ� �� �޽��� Ÿ�Կ� ���� �ݹ� �Լ��� �����մϴ�.
     * @param type ó���� �޽����� Ÿ�� (network::Message::Type).
     * @param handler �ش� Ÿ���� �޽����� �޾��� �� ȣ��� �Լ� ��ü.
     */
    void registerMessageHandler(network::Message::Type type, GenericMessageHandler handler);

private:
    network::MessageSender& messageSender_;     // �޽��� �۽ſ� ��ü
    network::MessageReceiver& messageReceiver_; // �޽��� ���ſ� ��ü
    service::ErrorService& errorService_;       // ���� ó���� ��ü

    std::string myVmId_;    // �� ���Ǳ��� ID
    int myCoordX_;          // �� ���Ǳ��� X ��ǥ
    int myCoordY_;          // �� ���Ǳ��� Y ��ǥ

    /**
     * @brief MessageReceiver�κ��� ��� Ÿ���� �޽����� �޾�,
     * ��ϵ� �ڵ鷯 ��(messageHandlers_)�� �����Ͽ� ������ �ڵ鷯�� ȣ���ϴ� ���� �ݹ�.
     * @param msg ���ŵ� network::Message ��ü.
     */
    void onMessageReceived(const network::Message& msg);

    // ��ϵ� �޽��� Ÿ�Ժ� �ڵ鷯���� �����ϴ� ��
    // Key: network::Message::Type, Value: GenericMessageHandler, Hash: network::EnumClassHash
    std::unordered_map<network::Message::Type, GenericMessageHandler, network::EnumClassHash> messageHandlers_;
};

} // namespace service
