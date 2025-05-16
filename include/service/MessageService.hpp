#pragma once
#include <string>
#include "domain/Drink.hpp"
#include "domain/Order.hpp"
#include "network/message.hpp"
#include <vector>         

namespace network { class MessageSender; class MessageReceiver; }
namespace persistence { class OvmAddressRepository; }

namespace application {

/**
 * MessageService
 * ?- MessageSender / MessageReceiver �� ����
 * ?  �޽��� �ۼ����� ����ϴ� ����.
 *
 * ?? ErrorService ������ TODO: �� ���ܵ�.
 */
class MessageService {
public:
    /**
     * ctor
     * @param sender   MessageSender (Network Layer)
     * @param receiver MessageReceiver (Network Layer)
     * @param repo     OvmAddressRepository ? VM ID �� endpoint ��ȸ��
     */
    MessageService(network::MessageSender&   sender,
                   network::MessageReceiver& receiver,
                   const persistence::OvmAddressRepository& repo);

    /* �������������������������������������� UC-8 �������������������������������������� */

    /**
     * broadcastStock(drink)
     * ?���� ���Ǳ��� ��� ��Ȳ(Drink) ��ȸ ��û�� ��� VM�� ��ε�ĳ��Ʈ�Ѵ�.
     * ?�� ���� �� �� VM�� RESP_STOCK ��ȯ
     */
    void broadcastStock(const domain::Drink& drink);

    /* �������������������������������������� UC-9 �������������������������������������� */

    /**
     * getDistance(list)
     * ?(���̾�׷������� DistanceService ���� ȣ��������)
     * ?��ε�ĳ��Ʈ ���� ����Ʈ�� �޾� ���� ����� VM�� ����ϴ� ���� �Լ�.
     */
     void getDistance(const std::vector<network::Message>& stockList);

    /* �������������������������������������� UC-15 / UC-16 �������������������������������������� */

    /**
     * sendPrePayReq(order)
     * ?T1 �� T2 ���� VM���� ������ ��û(REQ_PREPAY) ����
     */
    void sendPrePayReq(const domain::Order& order);

    /**
     * respondPrepayReq(order)
     * ?���� ��(T2)���� REQ_PREPAY ���� �� ȣ��.
     * ?��� Ȯ�� & PrePaymentCode ���� �� RESP_PREPAY �� �����Ѵ�.
     */
    void respondPrepayReq(const domain::Order& order);

    /* �������������������������������������� UC-17 �������������������������������������� */

    /**
     * validOVMStock(vm_id, drink, qty)
     * ?Ư�� VM(id)���� ��� �ִ��� Ȯ��.
     * ?REQ_STOCK / RESP_STOCK �պ� �� Bool ��ȯ.
     */
    bool validOVMStock(const std::string&   vm_id,
                       const domain::Drink& drink,
                       int                  qty = 1);

    /* �������������������������������������� ���� ��Ʈ�� ����Ʈ �������������������������������������� */

    /**
     * onMessage(msg)
     * ?MessageReceiver �� ������ ��� �޽�����
     * ?MessageService::onMessage �� ���� �� �������̽��� �б� ó��.
     */
    void onMessage(const network::Message& msg);

private:
    /* ���� ��ü */
    network::MessageSender&                 sender_;
    network::MessageReceiver&               receiver_;
    const persistence::OvmAddressRepository& repo_;

    /* ���� ���� */
    void handleReqStock (const network::Message& msg);
    void handleRespStock(const network::Message& msg);
    void handleReqPrepay(const network::Message& msg);
    void handleRespPrepay(const network::Message& msg);
};

} // namespace application
