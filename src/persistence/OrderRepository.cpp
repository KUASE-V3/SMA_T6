#include "persistence/OrderRepository.hpp" // OrderRepository Ŭ���� ���� ����
#include "domain/order.h" // domain::Order ���
#include <vector>
#include <string>
#include <algorithm> // std::find_if ���
#include <optional>  // findByCertCode���� ��ȯ Ÿ������ ��� ���� (����� Order ���� ��ȯ)

// OrderRepository.h ���� domain/Order.h �� �̹� ���ԵǾ� ������,
// .cpp ���Ͽ����� ���� ����ϴ� Ÿ�Կ� ���� ����� ��������� �����ϴ� ���� �����ϴ�.

namespace persistence {

// ���� �ڵ�� �ֹ� ��ȸ
domain::Order OrderRepository::findByCertCode(const std::string& certCode) {
    auto it = std::find_if(orders_.begin(), orders_.end(),
                           [&certCode](const domain::Order& order) {
                               return order.getCertCode() == certCode;
                           });

    if (it != orders_.end()) {
        return *it; // ã������ �ش� Order ��ü ��ȯ
    }
    // ã�� ������ ���, �� Order ��ü ��ȯ (�Ǵ� ���� ó�� ���)
    return domain::Order(); // �� Order ��ü ��ȯ

}

// �ֹ� ���� ����/������Ʈ
void OrderRepository::save(const domain::Order& order) {

    if (!order.getCertCode().empty()) {
        auto it = std::find_if(orders_.begin(), orders_.end(),
                               [&order](const domain::Order& o) {
                                   return o.getCertCode() == order.getCertCode();
                               });
        if (it != orders_.end()) {
            *it = order; // ������ certCode�� �ִ� �ֹ��̸� ������Ʈ
            return;
        }
    }
    // �׷��� �ʰų�, certCode�� ���� �ֹ��̸� ���� �߰�
    orders_.push_back(order);
}

// �ֹ� ���� ������Ʈ
bool OrderRepository::updateStatus(const std::string& vmid,
                                   const std::string& drinkCode,
                                   const std::string& certCode,
                                   const std::string& newStatus) {
    
    auto it = std::find_if(orders_.begin(), orders_.end(),
                           [&](const domain::Order& o) {
                               if (!certCode.empty()) {
                                   return o.getCertCode() == certCode;
                               }
                               // certCode�� ���� ���, ���� �ֱ��� PENDING ������ �ش� ���� �ֹ��� ã�´ٰ� ����.
                               return o.getVmid() == vmid &&
                                      o.getDrinkCode() == drinkCode &&
                                      o.getPayStatus() == "PENDING"; // ����: PENDING ������ �ֹ��� ������Ʈ
                           });

    if (it != orders_.end()) {
     
        (*it).setPayStatus(newStatus);
        return true;
    }
    return false; // ������Ʈ�� �ֹ��� ã�� ����
}


} // namespace persistence