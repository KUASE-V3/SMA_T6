

#include "service/DistanceService.hpp"
#include "domain/vendingMachine.h" // domain::VendingMachine 객체 생성 및 반환을 위해 필요
#include <algorithm> // std::sort
#include <vector>    // std::vector 사용
#include <utility>   // std::pair 사용 (candidatesWithDistance)
#include <stdexcept> // std::invalid_argument, std::out_of_range (extractNumericId에서 throw)

namespace service {


int DistanceService::extractNumericId(const std::string& vmId) const {
    if (vmId.length() > 1 && (vmId[0] == 'T' || vmId[0] == 't')) {
        try {
            // 'T' 다음 문자열부터 숫자 부분 추출
            return std::stoi(vmId.substr(1));
        } catch (const std::invalid_argument& ia) {
            // std::stoi가 숫자로 변환할 수 없는 문자열을 만났을 때
            throw std::invalid_argument("자판기 ID '" + vmId + "'에서 숫자 부분을 추출할 수 없습니다: " + ia.what());
        } catch (const std::out_of_range& oor) {
            // std::stoi가 변환한 숫자가 int 범위를 벗어났을 때
            throw std::out_of_range("자판기 ID '" + vmId + "'의 숫자 부분이 범위를 벗어났습니다: " + oor.what());
        }
    }
    // 'T'로 시작하지 않거나 길이가 너무 짧은 경우
    throw std::invalid_argument("유효하지 않은 자판기 ID 형식입니다: '" + vmId + "' (예: T1, T12)");
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
    candidatesWithDistance.reserve(otherVms.size()); // 메모리 할당 최적화

    // 1. 재고가 있는 자판기만 필터링하고, 거리를 계산하여 (거리, 자판기정보) 쌍으로 저장
    for (const auto& vmInfo : otherVms) {
        if (vmInfo.hasStock) { // 재고가 있는 자판기만 고려
            double dist = calculateDistance(currentVmX, currentVmY, vmInfo.coordX, vmInfo.coordY);
            candidatesWithDistance.emplace_back(dist, vmInfo);
        }
    }

    if (candidatesWithDistance.empty()) {
        return std::nullopt; // 재고가 있는 자판기가 하나도 없음
    }

    // 2. 거리 및 ID 기준으로 정렬 (PFR R3.2)
    //    - 1순위: 거리 오름차순
    //    - 2순위 (거리가 같을 시): ID의 숫자 부분 오름차순
    std::sort(candidatesWithDistance.begin(), candidatesWithDistance.end(),
        [this](const auto& a, const auto& b) { // 람다 비교 함수
            if (a.first < b.first) return true;  // 거리가 다르면 거리 기준
            if (a.first > b.first) return false;
            
            // 거리가 같은 경우, ID의 숫자 부분으로 비교
            try {
                return extractNumericId(a.second.id) < extractNumericId(b.second.id);
            } catch (const std::exception& e) {
                // ID 파싱 실패 시 단순 문자열 비교
                return a.second.id < b.second.id;
            }
        });

    // 3. 가장 우선순위가 높은 (가장 가깝고, ID 조건 만족하는) 자판기 정보 추출
    const auto& nearestVmInfo = candidatesWithDistance.front().second;

    // 4. domain::VendingMachine 객체로 변환하여 반환
    return domain::VendingMachine(nearestVmInfo.id, nearestVmInfo.coordX, nearestVmInfo.coordY, ""); // 포트 정보는 현재 알 수 없음
}

} // namespace service
