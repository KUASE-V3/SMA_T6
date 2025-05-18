#ifndef PREPAYMENTCODE_H
#define PREPAYMENTCODE_H

#include <string>
#include "order.h"

namespace domain {
class PrepaymentCode {
public:
    PrepaymentCode(); // 생성자

    std::string rand();          // 5자리 코드 생성
    PrepaymentCode hold(Order order);      // Order 객체 홀드
    static bool isUsable(const std::string& code);        //객체생성 없이 문자열 입력 후 메소드호출 가능



    std::string getCode() const;

    void setStatus(std ::string& status);

    private:
        std::string code;
        std::string status;
        Order heldOrder;
};
}
#endif