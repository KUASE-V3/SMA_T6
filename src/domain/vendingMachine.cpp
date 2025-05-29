#include "domain/vendingMachine.h" 
#include <string>                  // std::string 사용
#include <utility>                 // std::pair, std::make_pair 사용

namespace domain {

// VendingMachine.h 헤더 파일에 이미 인라인으로 구현되어 있습니다.
/*
VendingMachine::VendingMachine(std::string vmId, int x, int y, std::string port)
    : id_attribute(vmId),
      location_attribute(std::make_pair(x, y)),
      port_attribute(port) {}
*/

/*
std::string VendingMachine::getId() const { return id_attribute; }
std::pair<int, int> VendingMachine::getLocation() const { return location_attribute; }
std::string VendingMachine::getPort() const { return port_attribute; }
*/

/*
void VendingMachine::setLocation(int x, int y) {
    if (x >= 0 && x <= 99 && y >= 0 && y <= 99) { // 좌표 범위 0-99 [cite: PFR - DVM (202503) (1).pdf, page 3, source 18]
        location_attribute = std::make_pair(x,y);
    }
}

void VendingMachine::setPort(const std::string& newPort) {
    port_attribute = newPort;
}
*/


}