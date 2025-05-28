#ifndef ORDER_REPOSITORY_H
#define ORDER_REPOSITORY_H

#include <string>
#include <vector>
#include "domain/order.h" // Order Ŭ���� ���

namespace persistence {

class OrderRepository {
public:
    OrderRepository() = default;

    domain::Order findByCertCode(const std::string& certCode); // ������ �ֹ� ��ȸ��
    void save(const domain::Order& order); // �ֹ� ���� ����/������Ʈ
    // �ֹ� ���� ������Ʈ
    bool updateStatus(const std::string& vmid, const std::string& drinkCode, const std::string& certCode, const std::string& newStatus);
private:
    std::vector<domain::Order> orders_; // �޸� �� �ֹ� �����
    // �ֹ��� ���� �ڵ�(certCode)�� �ĺ��Ǹ�, ������ ���� �ڵ尡 �ִ� �ֹ��� ������Ʈ��.
    // certCode�� ���� ���, vmid�� drinkCode�� ���� �ֱ��� PENDING ������ �ֹ��� ã�� ������Ʈ��.
};

} // namespace persistence

#endif // ORDER_REPOSITORY_H