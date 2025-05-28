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
    // std::string orderId_attribute; // 고유 주문 ID (필요시 추가 고려)

public:
    Order(std::string vmid = "", std::string dCode = "", int qty = 1, std::string cCode = "", std::string pStatus = "PENDING")
        : vmid_attribute(vmid), drinkCode_attribute(dCode), qty_attribute(qty),
          certCode_attribute(cCode), paystatus_attribute(pStatus) {}

    // Getters
    std::string getVmid() const { return vmid_attribute; }
    std::string getDrinkCode() const { return drinkCode_attribute; }
    int getQty() const { return qty_attribute; }
    std::string getCertCode() const { return certCode_attribute; }
    std::string getPayStatus() const { return paystatus_attribute; }
    // std::string getOrderId() const { return orderId_attribute; }

    // Setters / Modifiers
    void setPayStatus(const std::string& newStatus) {
        // Potentially add validation for status values
        paystatus_attribute = newStatus;
    }
    void setCertCode(const std::string& newCertCode) {
        // 인증코드는 알파벳 대/소문자와 숫자를 포함하는 랜덤한 5자리의 문자열 
        // (여기서는 단순 할당, 생성은 서비스 계층에서)
        certCode_attribute = newCertCode;
    }
};
} // namespace domain
#endif // ORDER_H