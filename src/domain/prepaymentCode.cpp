#include "domain/prepaymentCode.h"
#include "domain/order.h" 
#include <memory>                  // std::shared_ptr 사용
#include <string>                  // std::string 사용

namespace domain {

//PrePaymentCode.h 헤더 파일에 이미 인라인으로 구현되어 있습니다.
/*
PrePaymentCode::PrePaymentCode(std::string code, CodeStatus status, std::shared_ptr<domain::Order> order)
    : code_attribute(code), status_attribute(status), heldOrder_attribute(order) {}
*/

/*
std::string PrePaymentCode::getCode() const { return code_attribute; }
CodeStatus PrePaymentCode::getStatus() const { return status_attribute; }
std::shared_ptr<domain::Order> PrePaymentCode::getHeldOrder() const { return heldOrder_attribute; }
*/

/*
bool PrePaymentCode::isUsable() const {
    return status_attribute == CodeStatus::ACTIVE;
}
*/

/*
void PrePaymentCode::markAsUsed() {
    status_attribute = CodeStatus::USED;
}
*/

/*
void PrePaymentCode::setHeldOrder(std::shared_ptr<domain::Order> order) {
    heldOrder_attribute = order;
}
*/


} // namespace domain