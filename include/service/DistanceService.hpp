#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cmath>
#include <stdexcept> // For std::invalid_argument (or custom exception)

// Forward declarations
namespace domain {
    class VendingMachine;
}
namespace service {
    class ErrorService; // 의존성 주입 고려 (만약 서비스 내부에서 직접 ErrorService 호출 시)
                        // 현재는 예외를 throw하고 호출 측에서 ErrorService 사용 가정
}

namespace service {

struct OtherVendingMachineInfo {
    std::string id;
    int coordX;
    int coordY;
    bool hasStock;
};

class DistanceService {
public:
    DistanceService(
        // service::ErrorService& errorService // 직접 ErrorService를 사용하지 않고 예외 throw
    ) {} // 생성자 내용은 .cpp 파일에서
    // ~DistanceService() = default; // 상속 고려 안 함

    std::optional<domain::VendingMachine> findNearestAvailableVendingMachine(
        int currentVmX,
        int currentVmY,
        const std::vector<OtherVendingMachineInfo>& otherVms
    ) const;

private:
    double calculateDistance(int x1, int y1, int x2, int y2) const {
        return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
    }

    /**
     * @brief 자판기 ID ("T1", "T2" 등)에서 숫자 부분 추출.
     * @param vmId "T"로 시작하고 뒤에 숫자가 오는 형식의 ID.
     * @return 추출된 숫자.
     * @throws std::invalid_argument (또는 사용자 정의 예외) ID 형식이 올바르지 않은 경우.
     * 이 예외는 ErrorService를 통해 처리될 수 있습니다.
     */
    int extractNumericId(const std::string& vmId) const;
};

} // namespace service