#include "prepaymentCode.h"
#include <cstdlib>     // rand, srand
#include <ctime>       // time
#include <sstream>     // stringstream
#include<string>
#include <cctype> // std::isalnum, std::isupper


namespace domain {
PrepaymentCode::PrepaymentCode() {
    code = rand();         // 5?���? ?��?�� 코드 ?��?��
    status = "Unused";     // 기본 ?��?��
    
}

// ?��?�� 5?���? 문자?�� ?��?�� ?��?��
std::string PrepaymentCode::rand() {
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


//?��증코?�� ?��?�� �??��
bool PrepaymentCode::isUsable( std::string& code) {
    if (code.length() != 5) return false;

    for (char c : code) {
        if (!std::isalnum(c)) return false;               // ?��?��?�� 문자 ?��?��경우,
        if (std::isalpha(c) && !std::isupper(c)) return false;      // ?��?��벳일 ?��, ?��문자?�� 경우 false
    }

    return true;
}


// ?��?��?�� 주문�? ?���?
PrepaymentCode PrepaymentCode::hold(Order order) {
    heldOrder = order;
    return *this;
}




//?��?��?��?���? �?�?
void PrepaymentCode::setStatus(std::string& newStatus){
    if (newStatus == "Used")    status = newStatus;
}



std::string PrepaymentCode :: getCode() const {
    return code;
}

}