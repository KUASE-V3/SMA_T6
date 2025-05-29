#include "persistence/OrderRepository.hpp" 
#include "domain/order.h" 
#include <vector>
#include <string>
#include <algorithm> 

namespace persistence {

// 인증 코드로 주문 조회
domain::Order OrderRepository::findByCertCode(const std::string& certCode) {
    auto it = std::find_if(orders_.begin(), orders_.end(),
                           [&certCode](const domain::Order& order) {
                               return order.getCertCode() == certCode;
                           });

    if (it != orders_.end()) {
        return *it; // 찾았으면 해당 Order 객체 반환
    }
    return domain::Order(); // 빈 Order 객체 반환

}

// 주문 정보 저장/업데이트
void OrderRepository::save(const domain::Order& order) {

    if (!order.getCertCode().empty()) {
        auto it = std::find_if(orders_.begin(), orders_.end(),
                               [&order](const domain::Order& o) {
                                   return o.getCertCode() == order.getCertCode();
                               });
        if (it != orders_.end()) {
            *it = order; // 동일한 certCode가 있는 주문이면 업데이트
            return;
        }
    }
    // 그렇지 않거나, certCode가 없는 주문이면 새로 추가
    orders_.push_back(order);
}

// 주문 상태 업데이트
bool OrderRepository::updateStatus(const std::string& vmid,
                                   const std::string& drinkCode,
                                   const std::string& certCode,
                                   const std::string& newStatus) {
    
    auto it = std::find_if(orders_.begin(), orders_.end(),
                           [&](const domain::Order& o) {
                               if (!certCode.empty()) {
                                   return o.getCertCode() == certCode;
                               }
                               // certCode가 없는 경우, 가장 최근의 PENDING 상태인 해당 음료 주문을 찾는다고 가정.
                               return o.getVmid() == vmid &&
                                      o.getDrinkCode() == drinkCode &&
                                      o.getPayStatus() == "PENDING"; // 예시: PENDING 상태의 주문만 업데이트
                           });

    if (it != orders_.end()) {
     
        (*it).setPayStatus(newStatus);
        return true;
    }
    return false; // 업데이트할 주문을 찾지 못함
}


} // namespace persistence