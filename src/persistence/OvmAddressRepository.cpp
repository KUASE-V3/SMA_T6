#include "persistence/OvmAddressRepository.hpp"
#include <vector> // for findAll

namespace persistence {

// OvmAddressRepository() = default; // 기본 생성자

// ID로 특정 외부 자판기 정보 조회
domain::VendingMachine OvmAddressRepository::findById(const std::string& vmId) {
    auto it = vms_.find(vmId);
    if (it != vms_.end()) {
        return it->second;
    }
    // 자판기를 찾지 못한 경우, 기본 생성된 VendingMachine 객체를 반환합니다.
    // 실제 운영 환경에서는 오류 처리 또는 예외 발생을 고려해야 합니다.
    // 예를 들어, UC9에서 응답을 수신하고, UC10에서 가장 가까운 자판기를 안내할 때
    // 해당 자판기의 상세 정보가 필요합니다[cite: 19, 22].
    // 만약 요청된 vmId가 없다면, 이는 시스템 설정 오류이거나 잘못된 ID일 수 있습니다.
    return domain::VendingMachine(); // 기본 객체 반환
}

// 모든 등록된 외부 자판기 정보 조회
std::vector<domain::VendingMachine> OvmAddressRepository::findAll() {
    std::vector<domain::VendingMachine> allVms;
    allVms.reserve(vms_.size());
    for (const auto& pair : vms_) {
        allVms.push_back(pair.second);
    }
    return allVms;
}

// 외부 자판기 정보 추가/업데이트 (초기 설정용 또는 동적 추가 시)
void OvmAddressRepository::save(const domain::VendingMachine& vm) {
    // vmId를 키로 사용하여 자판기 정보를 저장하거나 업데이트합니다.
    vms_[vm.getId()] = vm;
}

} // namespace persistence