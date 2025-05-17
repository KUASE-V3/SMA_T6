#pragma once
#include <string>
#include "domain/drink.h"   // Drink 정의

namespace domain {

/**
 * Order
 * ──────────────────────────────────────────
 * ? vm_id_      : 주문 대상 자판기 ID (ex: "T2")
 * ? drink_      : 음료(코드·이름·가격 포함)
 * ? qty_        : 수량  (지금은 1로만 사용해도 OK)
 * ? cert_code_  : 선결제 코드(없으면 "NONE")
 * ? pay_status_ : "Pending" | "Approved" | "Declined"
 *
 * ① 우리 자판기에서 주문 생성 → vmId 에 “내 자판기 ID” 저장  
 * ② 다른 자판기로 라우팅될 때 setVmId("Tn") 으로 목적지를 교체  
 * ③ 선결제 코드가 발급되면 attachPrePay 로 cert_code_ 채움
 */
class Order {
public:
    Order() = default;

    Order(std::string   vmId,
          Drink         drink,
          int           qty              = 1,
          std::string   certCode         = "NONE",
          std::string   payStatus        = "Pending");

    /* ─── 비즈니스 메서드 ─── */
    Order  attachPrePay(const std::string& cert);       // 선결제 코드 부착
    void   setStatus   (const std::string& payStatus);  // "Approved" | "Declined"
    void   setVmId     (const std::string& vmId);       // 목적지 수정

    /* ─── 게터 (MessageService 가 사용) ─── */
    const std::string& vmId()      const { return vm_id_; }
    const Drink&       drink()     const { return drink_; }
    int                quantity()  const { return qty_;   }
    const std::string& certCode()  const { return cert_code_; }

private:
    std::string vm_id_;      // 목적지 VM  (ex: "T2")
    Drink       drink_;
    int         qty_        {1};
    std::string cert_code_  {"NONE"};
    std::string pay_status_ {"Pending"};
};

} // namespace domain
