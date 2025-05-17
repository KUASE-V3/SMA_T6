#pragma once
/*������������������������������������������������������������������������������������������������������������������������������*
 *  MessageService
 *  ������������������������������
 *  ��Ʈ��ũ ����( MessageSender / MessageReceiver ) ���� �����
 *  VM �� �޽��� �ۡ����� ������ �߾����߽����� �����Ѵ�.
 *
 *  �� ���� �����������ǥ ��� ���� �ٸ� ������ ���񽺿� �����ϸ�,
 *     �� ���Ͽ��� ��ü ������ ���� �ʴ´�. (��� TODO ��Ŀ)
 *������������������������������������������������������������������������������������������������������������������������������*/

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <mutex>
#include <condition_variable>

#include "network/message.hpp"
#include "domain/drink.h"
#include "domain/order.h"


namespace network {
    class MessageSender;
    class MessageReceiver;
}
namespace persistence {
    class OvmAddressRepository;
}

namespace application {

class MessageService {
public:
    /*������������������������������������ ������ ������������������������������������*/
    MessageService(network::MessageSender&                 sender,
                   network::MessageReceiver&               receiver,
                   const persistence::OvmAddressRepository& repo);

    /*������������������������������������ UC-8 ������������������������������������*/
    /* ��� VM ���� ��� ��ȸ(REQ_STOCK) ��ε�ĳ��Ʈ */
    void broadcastStock(const domain::Drink& drink);
    /*������������������������������������ UC-9 ������������������������������������*/
    /* RESP_STOCK ����Ʈ�κ��� ���� ����� VM id ���� */
    std::optional<std::string>
        getDistance(const std::vector<network::Message>& stockList);

    /*������������������������������������ UC-15 / 16 ������������������������������������*/
    /* (T�� �� T��) ������ ��û */
    void sendPrePayReq(const domain::Order& order);

    /* (T��) ������ ��û ���� �� ���� */
    void respondPrepayReq(const domain::Order& order);

    /*������������������������������������ UC-17 ������������������������������������*/
    /* ���� VM �� ��� �ִ��� Ȯ�� (����) */
    bool validOVMStock(const std::string&   vm_id,
                       const domain::Drink& drink,
                       int                  qty = 1);

    /*������������������������������������ ���� ��Ʈ�� ������������������������������������*/
    void onMessage(const network::Message& msg);   // (����� subscribe �� �б�)

private:
    /* ��Ʈ��ũ ���� */
    network::MessageSender&                   sender_;
    network::MessageReceiver&                 receiver_;
    const persistence::OvmAddressRepository&  repo_;

    /* ��ε�ĳ��Ʈ ���� ĳ�� */
    std::mutex                                resp_mtx_;
    std::vector<network::Message>             resp_cache_;
    std::condition_variable                   resp_cv_;      // �� �ʿ� �� ���

    /* ���� ���� */
    void handleReqStock (const network::Message& msg);
    void handleRespStock(const network::Message& msg);
    void handleReqPrepay(const network::Message& msg);
    void handleRespPrepay(const network::Message& msg);
};

} // namespace application
