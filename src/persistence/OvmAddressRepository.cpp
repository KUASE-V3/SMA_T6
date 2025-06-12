#include "persistence/OvmAddressRepository.hpp"
#include <vector> // for findAll

namespace persistence {


// ID로 특정 외부 자판기 정보 조회
domain::VendingMachine OvmAddressRepository::findById(const std::string& vmId) const{
    auto it = vms_.find(vmId);
    if (it != vms_.end()) {
        return it->second;
    }
    return domain::VendingMachine(); // 기본 객체 반환
}

// 모든 등록된 외부 자판기 정보 조회
std::vector<domain::VendingMachine> OvmAddressRepository::findAll() const {
    std::vector<domain::VendingMachine> allVms;
    allVms.reserve(vms_.size());
    // pair 대신 [id, vm]으로 바로 분해합니다. id는 사용하지 않지만 명시적으로 보여줍니다.
    for (const auto& [id, vm] : vms_) {
        allVms.push_back(vm);
    }
    return allVms;
}

// 외부 자판기 정보 추가/업데이트 (초기 설정용 또는 동적 추가 시)
void OvmAddressRepository::save(const domain::VendingMachine& vm) {
    vms_[vm.getId()] = vm;
}

} // namespace persistence