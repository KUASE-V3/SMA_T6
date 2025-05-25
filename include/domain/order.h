
#pragma once
#include <string>
#include "domain/drink.h"   


namespace domain {
class Order {
public:

    Order(std::string   vmId            = "T1",    //선결제이고, 코드 생성시 해당 자판기의 ID를 넣어야함 -> vmid값을 받아오는 과정을 몰라서 임시로 디폴트 값 배정
          Drink         drink           = Drink(),
          int           qty              = 1,
          std::string   certCode         = "NONE", // 인증코드 초기값은 NONE
          std::string   payStatus        = "Pending"); // 결제 상태 초기값은 Pending, 1차 때는 결제 상태를 나타내는 클래스를 정의하지 않고 문자열로 처리합니다.

    Order  attachPrePay(const std::string& drinkID, const std::string& code);       
    void   setStatus   (const std::string& payStatus);  
    void   setVmId     (const std::string& vmId);     

    const std::string& vmId()      const { return vm_id_; }
    const Drink&       drink()     const { return drink_; }
    int                quantity()  const { return qty_;   }
    const std::string& certCode()  const { return cert_code_; }

private:
    std::string vm_id_      {"T1"};
    Drink       drink_;
    int         qty_        {1}; // 음료 수량 현재는 1개로 고정 
    std::string cert_code_  {"NONE"};
    std::string pay_status_ {"Pending"};
};

}