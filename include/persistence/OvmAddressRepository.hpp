#ifndef OVM_ADDRESS_REPOSITORY_HPP
#define OVM_ADDRESS_REPOSITORY_HPP

#include <string>
#include <vector>
#include <map> // 재고 저장을 위해 사용

#include "domain/vendingMachine.h" // VendingMachine 클래스 사용

namespace persistence {

class OvmAddressRepository {
public:
    OvmAddressRepository() = default;

    // ID로 특정 외부 자판기 정보 조회
    domain::VendingMachine findById(const std::string& vmId);
    // 모든 등록된 외부 자판기 정보 조회
    std::vector<domain::VendingMachine> findAll();
    // 외부 자판기 정보 추가/업데이트 (초기 설정용)
    void save(const domain::VendingMachine& vm);
private:
    std::map<std::string, domain::VendingMachine> vms_;
};

} // namespace persistence

#endif // OVM_ADDRESS_REPOSITORY_HPP