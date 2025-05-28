#ifndef ORDER_REPOSITORY_H
#define ORDER_REPOSITORY_H

#include <string>
#include <vector>
#include "domain/order.h" // Order 클래스 사용

namespace persistence {

class OrderRepository {
public:
    OrderRepository() = default;

    domain::Order findByCertCode(const std::string& certCode); // 선결제 주문 조회용
    void save(const domain::Order& order); // 주문 정보 저장/업데이트
    // 주문 상태 업데이트
    bool updateStatus(const std::string& vmid, const std::string& drinkCode, const std::string& certCode, const std::string& newStatus);
private:
    std::vector<domain::Order> orders_; // 메모리 내 주문 저장소
    // 주문은 인증 코드(certCode)로 식별되며, 동일한 인증 코드가 있는 주문은 업데이트됨.
    // certCode가 없는 경우, vmid와 drinkCode로 가장 최근의 PENDING 상태인 주문을 찾아 업데이트함.
};

} // namespace persistence

#endif // ORDER_REPOSITORY_H