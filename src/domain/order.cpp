#include "domain/order.h" // Order Ŭ���� ���Ǹ� ����
#include <string> // setPayStatus, setCertCode ��� std::string ���

namespace domain {

// Order.h ��� ���Ͽ� �̹� �ζ������� �����Ǿ� �ֽ��ϴ�.
/*
Order::Order(std::string vmid, std::string dCode, int qty, std::string cCode, std::string pStatus)
    : vmid_attribute(vmid),
      drinkCode_attribute(dCode),
      qty_attribute(qty),
      certCode_attribute(cCode),
      paystatus_attribute(pStatus) {
    // �⺻������ qty=1, pStatus="PENDING" ���� ����� ��õ�
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