#ifndef VENDINGMACHINE_H
#define VENDINGMACHINE_H

#include <string>
#include <utility>
namespace domain {

// 초기화시 : id, location을 "T5", {123, 123}, "8080"으로 설정합니다.
class VendingMachine {
    public:
    VendingMachine(const std::string& id = "T5", const std::pair<int, int>& location = {123, 123}, const std::string& port = "8080");

    std::string getId() const;

    std::pair<int, int> getLocation() const;

    std::string getPort() const;


    private:
        std::string id; //자판기 ID, 5조이므로 T5로 설정
        std::pair<int, int> location; //좌표쌍
        std::string port; //포트번호
    };
    
}

#endif
