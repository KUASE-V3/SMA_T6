#pragma once

#include <string>
#include <vector>
#include <optional> // std::optional 사용
#include <cmath>    // std::sqrt, std::pow 사용
#include <stdexcept> // std::invalid_argument 등 예외 클래스 사용

namespace domain {
    class VendingMachine; // domain::VendingMachine 타입 반환을 위해 전방 선언
}

namespace service {

/**
 * @brief 다른 자판기들의 정보를 담는 구조체.
 * UserProcessController가 MessageService로부터 응답을 받아 이 구조체의 벡터를 구성하고,
 * DistanceService에 전달하여 가장 가까운 자판기를 찾는데 사용됩니다.
 */
struct OtherVendingMachineInfo {
    std::string id;     ///< 다른 자판기의 ID
    int coordX;         ///< 다른 자판기의 X 좌표
    int coordY;         ///< 다른 자판기의 Y 좌표
    bool hasStock;      ///< 해당 자판기에 원하는 음료의 재고가 있는지 여부
};

/**
 * @brief 자판기 간의 거리를 계산하고, 조건에 맞는 가장 가까운 자판기를 찾는 기능을 제공하는 서비스입니다.
 * 주로 UC10 (가장 가까운 자판기 안내)에서 사용됩니다.
 */
class DistanceService {
public:
    /**
     * @brief DistanceService 생성자.
     * 현재는 특별한 초기화 로직이 없습니다.
     */
    DistanceService() = default; 

    /**
     * @brief 주어진 다른 자판기 정보 목록 중에서 현재 자판기를 기준으로 가장 가깝고,
     * 재고가 있는 자판기를 찾아 반환합니다. (UC10, PFR R3.2)
     * 거리가 같을 경우 자판기 ID의 숫자 부분이 작은 것을 우선합니다.
     * @param currentVmX 현재 자판기의 X 좌표.
     * @param currentVmY 현재 자판기의 Y 좌표.
     * @param otherVms 다른 자판기들의 정보(ID, 좌표, 재고 유무)를 담은 벡터.
     * @return 조건에 맞는 가장 가까운 자판기의 domain::VendingMachine 객체.
     * 조건에 맞는 자판기가 없으면 std::nullopt를 반환합니다.
     */
    std::optional<domain::VendingMachine> findNearestAvailableVendingMachine(
        int currentVmX,
        int currentVmY,
        const std::vector<OtherVendingMachineInfo>& otherVms
    ) const;

private:
    /**
     * @brief 두 좌표 간의 유클리드 직선 거리를 계산합니다.
     * @param x1 첫 번째 지점의 X 좌표.
     * @param y1 첫 번째 지점의 Y 좌표.
     * @param x2 두 번째 지점의 X 좌표.
     * @param y2 두 번째 지점의 Y 좌표.
     * @return 두 지점 간의 거리.
     */
    double calculateDistance(int x1, int y1, int x2, int y2) const {
        // PFR 1.2: "매번 통신을 주고받을 때마다 자판기의 좌표를 받아와 직선 거리를 계산한다."
        double dx = static_cast<double>(x1) - static_cast<double>(x2);
        double dy = static_cast<double>(y1) - static_cast<double>(y2);
        return std::sqrt(std::pow(dx, 2) + std::pow(dy, 2));
    }

    /**
     * @brief 자판기 ID (예: "T1", "T12")에서 숫자 부분을 추출하여 정수로 반환합니다.
     * ID는 'T' 또는 't'로 시작하고 그 뒤에 숫자가 오는 형식을 가정합니다.
     * (PFR R3.2: "거리가 같다면 id의 숫자가 작은 자판기로 안내한다.")
     * @param vmId 추출할 자판기의 ID 문자열.
     * @return 추출된 숫자.
     * @throws std::invalid_argument ID 형식이 올바르지 않거나 숫자 변환에 실패한 경우.
     */
    int extractNumericId(const std::string& vmId) const;
};

} // namespace service


