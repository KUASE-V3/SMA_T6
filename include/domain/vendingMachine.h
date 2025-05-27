#ifndef VENDINGMACHINE_H
#define VENDINGMACHINE_H

#include <string>
#include <utility>
namespace domain {

// �ʱ�ȭ�� : id, location�� "T5", {123, 123}, "8080"���� �����մϴ�.
class VendingMachine {
    public:
    VendingMachine(const std::string& id = "T5", const std::pair<int, int>& location = {123, 123}, const std::string& port = "8080");

    std::string getId() const;

    std::pair<int, int> getLocation() const;

    std::string getPort() const;


    private:
        std::string id; //���Ǳ� ID, 5���̹Ƿ� T5�� ����
        std::pair<int, int> location; //��ǥ��
        std::string port; //��Ʈ��ȣ
    };
    
}
#endif
