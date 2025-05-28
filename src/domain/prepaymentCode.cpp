#include "domain/prepaymentCode.h"

#include <cstdlib>     
#include <ctime>       // time
#include <sstream>     // stringstream
#include<string>
#include <cctype> // std::isalnum, std::isupper


using namespace domain;

PrepaymentCode::PrepaymentCode() {
<<<<<<< HEAD
    code = rand();         
    status = "Unused";     
=======
    code = generate();         // 5?? ?? 코드 ??
    status = "Unused";     // 기본 ??
>>>>>>> main
    
}

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


bool PrepaymentCode::isUsable( const std::string& code) {
    if (code.length() != 5) return false;

    for (char c : code) {
        if (!std::isalnum(c)) return false;               
        if (std::isalpha(c) && !std::isupper(c)) return false;     
    }

    return true;
}



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



std::string PrepaymentCode :: getStatus() {
    return status;
}