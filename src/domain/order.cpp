#include "domain/order.h"
#include "persistence/inventoryRepository.h"


namespace domain {


Order::Order(std::string  vmId,
             Drink        drink,
             int          qty,
             std::string  certCode,
             std::string  payStatus)        //기본값 : Pending
    : vm_id_(std::move(vmId))
    , drink_(std::move(drink))
    , qty_(qty)
    , cert_code_(std::move(certCode))
    , pay_status_(std::move(payStatus))
{}

Order Order::attachPrePay(const std::string& drinkID, const std::string& code) { //선결제로 전환시 

    for (const auto& inventory : persistence::inventoryRepository::getAllDrinks()) {                //드링크 id를 레퍼지토리 리스트에서 찾아서 드링크 객체를 가져옴
        if (inventory.getDrink().getCode() == drinkID) {
            
            return Order(vm_id_, inventory.getDrink(), qty_, code, pay_status_);
        }
    }
    //찾지못했을 경우
    return Order(vm_id_, Drink(), qty_, code, pay_status_);

}

void Order::setStatus(const std::string& status) { 
    if (status == "Approved" || status == "Declined")
        pay_status_ = status;
}

void Order::setVmId(const std::string& vmId) {
    vm_id_ = vmId;
}






}