
#pragma once
#include <string>
#include "domain/drink.h"   // Drink ����

namespace domain {

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
