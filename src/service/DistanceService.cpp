#include "service/DistanceService.hpp"
#include "domain/vendingMachine.h" // domain::VendingMachine 사용을 위해 필요
#include <algorithm> // std::sort, std::min_element 등
#include <limits>    // std::numeric_limits
#include <vector>    // std::vector 임시 사용

namespace service {

// DistanceService() 생성자는 헤더에서 이미 {} 로 정의됨

/**
 * @brief 자판기 ID ("T1", "T2" 등)에서 숫자 부분 추출.
 * @param vmId "T"로 시작하고 뒤에 숫자가 오는 형식의 ID.
 * @return 추출된 숫자.
 * @throws std::invalid_argument ID 형식이 올바르지 않은 경우.
 */
int DistanceService::extractNumericId(const std::string& vmId) const {
    if (vmId.length() > 1 && (vmId[0] == 'T' || vmId[0] == 't')) {
        try {
            return std::stoi(vmId.substr(1));
        } catch (const std::invalid_argument& ia) {
            throw std::invalid_argument("자판기 ID에서 숫자 부분을 추출할 수 없습니다: " + vmId + " (" + ia.what() + ")");
        } catch (const std::out_of_range& oor) {
            throw std::out_of_range("자판기 ID의 숫자 부분이 범위를 벗어났습니다: " + vmId + " (" + oor.what() + ")");
        }
    }
    throw std::invalid_argument("유효하지 않은 자판기 ID 형식입니다: " + vmId);
}

std::optional<domain::VendingMachine> DistanceService::findNearestAvailableVendingMachine(
    int currentVmX,
    int currentVmY,
    const std::vector<OtherVendingMachineInfo>& otherVms
) const {
    if (otherVms.empty()) {
        return std::nullopt; // 후보 자판기가 없음
    }

    std::vector<std::pair<double, OtherVendingMachineInfo>> candidatesWithDistance;

    // 재고가 있는 자판기만 필터링하고, 거리를 계산하여 저장
    for (const auto& vmInfo : otherVms) {
        if (vmInfo.hasStock) {
            double dist = calculateDistance(currentVmX, currentVmY, vmInfo.coordX, vmInfo.coordY);
            candidatesWithDistance.push_back({dist, vmInfo});
        }
    }

    if (candidatesWithDistance.empty()) {
        return std::nullopt; // 재고가 있는 자판기가 없음
    }

    // 거리 및 ID 기준으로 정렬
    std::sort(candidatesWithDistance.begin(), candidatesWithDistance.end(),
        [this](const auto& a, const auto& b) {
            if (a.first != b.first) { // 거리 비교
                return a.first < b.first;
            }
            // 거리가 같으면 ID의 숫자 부분으로 비교 [cite: 89]
            try {
                return extractNumericId(a.second.id) < extractNumericId(b.second.id);
            } catch (const std::exception& e) {
                // ID 파싱 실패 시 예외 처리 (예: 로그 남기기) 또는 단순 문자열 비교로 대체
                // 여기서는 단순 문자열 비교로 폴백
                return a.second.id < b.second.id;
            }
        });

    // 가장 가까운 자판기 정보로 domain::VendingMachine 객체 생성
    const auto& nearestVmInfo = candidatesWithDistance.front().second;
    // domain::VendingMachine은 id, location(pair), port를 가짐.
    // OtherVendingMachineInfo에는 port 정보가 없으므로, 포트 정보는 비워두거나,
    // OvmAddressRepository 등을 통해 조회해야 할 수 있음.
    // 현재 헤더로는 포트 정보를 알 수 없으므로 기본값으로 설정.
    return domain::VendingMachine(nearestVmInfo.id, nearestVmInfo.coordX, nearestVmInfo.coordY, "");
}

} // namespace service