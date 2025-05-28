#include "persistence/OrderRepository.hpp" // OrderRepository 클래스 정의 포함
#include "domain/order.h" // domain::Order 사용
#include <vector>
#include <string>
#include <algorithm> // std::find_if 사용
#include <optional>  // findByCertCode에서 반환 타입으로 고려 가능 (현재는 Order 직접 반환)

// OrderRepository.h 에는 domain/Order.h 가 이미 포함되어 있지만,
// .cpp 파일에서도 직접 사용하는 타입에 대한 헤더를 명시적으로 포함하는 것이 좋습니다.

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
    // 찾지 못했을 경우, 빈 Order 객체 반환 (또는 예외 처리 고려)
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