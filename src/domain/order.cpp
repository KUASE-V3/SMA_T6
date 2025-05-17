#include "domain/order.h"

namespace domain {

/* 기본 생성자: dummy 주문 */
Order::Order(std::string  vmId,
             Drink        drink,
             int          qty,
             std::string  certCode,
             std::string  payStatus)
    : vm_id_(std::move(vmId))
    , drink_(std::move(drink))
    , qty_(qty)
    , cert_code_(std::move(certCode))
    , pay_status_(std::move(payStatus))
{}

/* 선결제 코드 부착 → 새 Order 반환 (불변 객체 스타일) */
Order Order::attachPrePay(const std::string& cert) {
    return Order(vm_id_, drink_, qty_, cert, pay_status_);
}

/* 결제 결과 저장 */
void Order::setStatus(const std::string& status) {
    if (status == "Approved" || status == "Declined")
        pay_status_ = status;
}

/* 메시지 라우팅 단계에서 목적지 VM 을 바꿀 수 있게 제공 */
void Order::setVmId(const std::string& vmId) {
    vm_id_ = vmId;
}

} // namespace domain
