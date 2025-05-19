#include "domain/order.h"

namespace domain {

Order::Order(std::string  vmId,
             Drink        drink,
             int          qty,
             std::string  certCode,
             std::string  payStatus)
    : vm_id_(std::move(vmId))
    , drink_(std::move(drink))
    , qty_(qty)
    , cert_code_(std::move(certCode))
    , pay_status_(std::move(payStatus))
{}

Order Order::attachPrePay(const std::string& cert) { //선결제로 전환시 
    return Order(vm_id_, drink_, qty_, cert, pay_status_);
}

void Order::setStatus(const std::string& status) { 
    if (status == "Approved" || status == "Declined")
        pay_status_ = status;
}

void Order::setVmId(const std::string& vmId) {
    vm_id_ = vmId;
}





}