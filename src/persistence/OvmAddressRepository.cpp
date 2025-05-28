#include "persistence/OvmAddressRepository.hpp"
#include <vector> // for findAll

namespace persistence {

// OvmAddressRepository() = default; // �⺻ ������

// ID�� Ư�� �ܺ� ���Ǳ� ���� ��ȸ
domain::VendingMachine OvmAddressRepository::findById(const std::string& vmId) {
    auto it = vms_.find(vmId);
    if (it != vms_.end()) {
        return it->second;
    }
    // ���Ǳ⸦ ã�� ���� ���, �⺻ ������ VendingMachine ��ü�� ��ȯ�մϴ�.
    // ���� � ȯ�濡���� ���� ó�� �Ǵ� ���� �߻��� ����ؾ� �մϴ�.
    // ���� ���, UC9���� ������ �����ϰ�, UC10���� ���� ����� ���Ǳ⸦ �ȳ��� ��
    // �ش� ���Ǳ��� �� ������ �ʿ��մϴ�[cite: 19, 22].
    // ���� ��û�� vmId�� ���ٸ�, �̴� �ý��� ���� �����̰ų� �߸��� ID�� �� �ֽ��ϴ�.
    return domain::VendingMachine(); // �⺻ ��ü ��ȯ
}

// ��� ��ϵ� �ܺ� ���Ǳ� ���� ��ȸ
std::vector<domain::VendingMachine> OvmAddressRepository::findAll() {
    std::vector<domain::VendingMachine> allVms;
    allVms.reserve(vms_.size());
    for (const auto& pair : vms_) {
        allVms.push_back(pair.second);
    }
    return allVms;
}

// �ܺ� ���Ǳ� ���� �߰�/������Ʈ (�ʱ� ������ �Ǵ� ���� �߰� ��)
void OvmAddressRepository::save(const domain::VendingMachine& vm) {
    // vmId�� Ű�� ����Ͽ� ���Ǳ� ������ �����ϰų� ������Ʈ�մϴ�.
    vms_[vm.getId()] = vm;
}

} // namespace persistence