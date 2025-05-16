#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace persistence {

// OvmAddressRepository: Other Vending Machine 주소(호스트:포트) 매핑을 제공
// - 10개 정도 미리 예시 작성할거임
class OvmAddressRepository {
public:
    OvmAddressRepository();

    // OVM ID -> "host:port" 반환 (없으면 빈 문자열)
    std::string getEndpoint(const std::string& id) const;

    // Broadcast용 전체 endpoint 목록 반환
    std::vector<std::string> getAllEndpoints() const;

private:
    std::unordered_map<std::string, std::string> id_map_;
};

} // namespace persistence
