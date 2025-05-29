#include "domain/prepaymentCode.h"

#include <cstdlib>     // rand, srand
#include <ctime>       // time
#include <sstream>     // stringstream
#include<string>
#include <cctype> // std::isalnum, std::isupper


using namespace domain;

PrepaymentCode::PrepaymentCode() {
    code = generate();         // 5?? ?? 코드 ??
    status = "Unused";     // 기본 ??
    
}

// ?? 5?? 문자? ?? ??
std::string PrepaymentCode::generate() {
    static const char charset[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";

    std::string result;
    for (int i = 0; i < 5; ++i) {
        int index = std::rand() % (sizeof(charset) - 1);
        result += charset[index];
    }

    return result;
}


//?증코? ????
bool PrepaymentCode::isUsable( const std::string& code) {
    if (code.length() != 5) return false;

    for (char c : code) {
        if (!std::isalnum(c)) return false;               // ??? 문자 ??경우,
        if (std::isalpha(c) && !std::isupper(c)) return false;      // ??벳일 ?, ?문자? 경우 false
    }

    return true;
}



// ??? 주문 ????? ?증코? prepaymentcode??? ??
PrepaymentCode PrepaymentCode::hold(Order order) {
    heldOrder = order;
    return *this;               //prepaymentcode 반환. 후에 레퍼지토리에 order를 붙인 code객체를 저장할 용도
}




//코드 사용
void PrepaymentCode::use(const std::string& code) {
    if (this->code == code) {
        status = "Used";
    }
}



std::string PrepaymentCode :: getCode() const {
    return code;
}
*/

/*
void PrePaymentCode::setHeldOrder(std::shared_ptr<domain::Order> order) {
    heldOrder_attribute = order;
}
*/


} // namespace domain