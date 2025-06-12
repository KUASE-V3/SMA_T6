#ifndef OVM_ADDRESS_REPOSITORY_HPP
#define OVM_ADDRESS_REPOSITORY_HPP

#include <string>
#include <vector>
#include <map> // 재고 저장을 위해 사용

#include "domain/vendingMachine.h" 

namespace persistence {

class OvmAddressRepository {
public:
    OvmAddressRepository() = default;

    domain::VendingMachine findById(const std::string& vmId) const;
    std::vector<domain::VendingMachine> findAll() const;
    void save(const domain::VendingMachine& vm);
private:
    std::map<std::string, domain::VendingMachine> vms_;
};

} // namespace persistence

#endif // OVM_ADDRESS_REPOSITORY_HPP