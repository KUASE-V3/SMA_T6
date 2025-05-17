#pragma once
#include <string>
#include "domain/drink.h"   // Drink ����

namespace domain {

/**
 * Order
 * ������������������������������������������������������������������������������������
 * ? vm_id_      : �ֹ� ��� ���Ǳ� ID (ex: "T2")
 * ? drink_      : ����(�ڵ塤�̸������� ����)
 * ? qty_        : ����  (������ 1�θ� ����ص� OK)
 * ? cert_code_  : ������ �ڵ�(������ "NONE")
 * ? pay_status_ : "Pending" | "Approved" | "Declined"
 *
 * �� �츮 ���Ǳ⿡�� �ֹ� ���� �� vmId �� ���� ���Ǳ� ID�� ����  
 * �� �ٸ� ���Ǳ�� ����õ� �� setVmId("Tn") ���� �������� ��ü  
 * �� ������ �ڵ尡 �߱޵Ǹ� attachPrePay �� cert_code_ ä��
 */
class Order {
public:
    Order() = default;

    Order(std::string   vmId,
          Drink         drink,
          int           qty              = 1,
          std::string   certCode         = "NONE",
          std::string   payStatus        = "Pending");

    /* ������ ����Ͻ� �޼��� ������ */
    Order  attachPrePay(const std::string& cert);       // ������ �ڵ� ����
    void   setStatus   (const std::string& payStatus);  // "Approved" | "Declined"
    void   setVmId     (const std::string& vmId);       // ������ ����

    /* ������ ���� (MessageService �� ���) ������ */
    const std::string& vmId()      const { return vm_id_; }
    const Drink&       drink()     const { return drink_; }
    int                quantity()  const { return qty_;   }
    const std::string& certCode()  const { return cert_code_; }

private:
    std::string vm_id_;      // ������ VM  (ex: "T2")
    Drink       drink_;
    int         qty_        {1};
    std::string cert_code_  {"NONE"};
    std::string pay_status_ {"Pending"};
};

} // namespace domain
