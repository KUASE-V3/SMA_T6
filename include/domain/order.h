#ifndef ORDER_H
#define ORDER_H

#include <string>

namespace domain {
class Order {
private:
    std::string vmid_attribute;      // 주문이 발생한/처리될 자판기 ID
    std::string drinkCode_attribute; // 주문한 음료의 코드
    int qty_attribute;               // 주문 수량 (항상 1)
    std::string certCode_attribute;  // 선결제 시 발급된 인증코드
    std::string paystatus_attribute; // 결제 상태 (예: "PENDING", "APPROVED", "DECLINED")

public:
    Order(const std::string&  vmid = "", const std::string& dCode  = "", int qty = 1, const std::string&  cCode = "", const std::string&  pStatus = "PENDING")
        : vmid_attribute(vmid), drinkCode_attribute(dCode), qty_attribute(qty),
          certCode_attribute(cCode), paystatus_attribute(pStatus) {}

    // Getters
    std::string getVmid() const { return vmid_attribute; }
    std::string getDrinkCode() const { return drinkCode_attribute; }
    int getQty() const { return qty_attribute; }
    std::string getCertCode() const { return certCode_attribute; }
    std::string getPayStatus() const { return paystatus_attribute; }

    // Setters / Modifiers
    void setPayStatus(const std::string& newStatus) {
        paystatus_attribute = newStatus;
    }
    void setCertCode(const std::string& newCertCode) {
        certCode_attribute = newCertCode;
    }
};
} // namespace domain
#endif // ORDER_H