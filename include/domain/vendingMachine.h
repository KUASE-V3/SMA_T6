#ifndef VENDING_MACHINE_H
#define VENDING_MACHINE_H

#include <string>
#include <utility> // For std::pair

namespace domain {
class VendingMachine {
private:
    std::string id_attribute;                // 자판기 ID (예: "T6")
    std::pair<int, int> location_attribute;  // 자판기 위치 <x, y> (0~99) [cite: 18]
    std::string port_attribute;              // 자판기 네트워크 포트 정보

public:
    VendingMachine(std::string vmId = "", int x = 0, int y = 0, std::string port = "")
        : id_attribute(vmId), location_attribute(std::make_pair(x, y)), port_attribute(port) {}

    // Getters
    std::string getId() const { return id_attribute; }
    std::pair<int, int> getLocation() const { return location_attribute; }
    std::string getPort() const { return port_attribute; }

    // Setters (주로 초기화 시 사용)
    void setLocation(int x, int y) {
        if (x >= 0 && x <= 99 && y >= 0 && y <= 99) { 
            location_attribute = std::make_pair(x,y);
        }
        // Optionally, handle invalid coordinates (e.g., throw exception, log error)
    }
    void setPort(const std::string& newPort) { port_attribute = newPort; }
};
}
#endif // VENDING_MACHINE_H