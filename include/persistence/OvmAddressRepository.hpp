#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace persistence {

// OvmAddressRepository: Other Vending Machine �ּ�(ȣ��Ʈ:��Ʈ) ������ ����
// - 10�� ���� �̸� ���� �ۼ��Ұ���
class OvmAddressRepository {
public:
    OvmAddressRepository();

    // OVM ID -> "host:port" ��ȯ (������ �� ���ڿ�)
    std::string getEndpoint(const std::string& id) const;

    // Broadcast�� ��ü endpoint ��� ��ȯ
    std::vector<std::string> getAllEndpoints() const;

private:
    std::unordered_map<std::string, std::string> id_map_;
};

} // namespace persistence
