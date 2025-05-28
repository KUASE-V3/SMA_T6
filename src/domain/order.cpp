#include "domain/order.h" // Order 클래스 정의를 포함
#include <string> // setPayStatus, setCertCode 등에서 std::string 사용

namespace domain {

// Order.h 헤더 파일에 이미 인라인으로 구현되어 있습니다.
/*
Order::Order(std::string vmid, std::string dCode, int qty, std::string cCode, std::string pStatus)
    : vmid_attribute(vmid),
      drinkCode_attribute(dCode),
      qty_attribute(qty),
      certCode_attribute(cCode),
      paystatus_attribute(pStatus) {
    // 기본값으로 qty=1, pStatus="PENDING" 등이 헤더에 명시됨
}
*/

/*
std::string Order::getVmid() const { return vmid_attribute; }
std::string Order::getDrinkCode() const { return drinkCode_attribute; }
int Order::getQty() const { return qty_attribute; }
std::string Order::getCertCode() const { return certCode_attribute; }
std::string Order::getPayStatus() const { return paystatus_attribute; }
*/

/*
void Order::setPayStatus(const std::string& newStatus) {
    paystatus_attribute = newStatus;
}

void Order::setCertCode(const std::string& newCertCode) {
    certCode_attribute = newCertCode;
}
*/


} // namespace domain