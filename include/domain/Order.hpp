#pragma once
#include <string>
#include "domain/Drink.hpp"

//�׽�Ʈ ���� �����ڵ���(������ �� �����ؾ���), ���� ���� ��� ����
namespace domain {

class Order {
public:
    Order(std::string vmId, Drink d, int q, std::string cert = "NONE")
      : vm_id_(std::move(vmId)), drink_(std::move(d)), qty_(q), cert_(std::move(cert)) {}

    const std::string& vm_id()    const { return vm_id_; }
    const Drink&       drink()    const { return drink_; }
    const std::string& drinkCode()const { return drink_.code(); }
    int                qty()      const { return qty_; }
    const std::string& certCode() const { return cert_; }

private:
    std::string vm_id_;
    Drink       drink_;
    int         qty_;
    std::string cert_;
};

} // namespace domain
