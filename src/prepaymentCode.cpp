#include "prepaymentCode.h"
#include <cstdlib>     // rand, srand
#include <ctime>       // time
#include <sstream>     // stringstream
#include<string>
#include <cctype> // std::isalnum, std::isupper


PrepaymentCode::PrepaymentCode() {
    code = rand();         // 5자리 랜덤 코드 생성
    status = "Unused";     // 기본 상태
    
}

// 랜덤 5자리 문자열 생성 함수
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


//인증코드 형식 검사
bool PrepaymentCode::isUsable( std::string& code) {
    if (code.length() != 5) return false;

    for (char c : code) {
        if (!std::isalnum(c)) return false;               // 숫자나 문자 아닌경우,
        if (std::isalpha(c) && !std::isupper(c)) return false;      // 알파벳일 때, 소문자일 경우 false
    }

    return true;
}


// 들어온 주문과 연결
void PrepaymentCode::hold(Order order) {
    heldOrder = order;
}





//코드반환
std::string PrepaymentCode :: getCode() const {
    return code;
}

//상태반환
std::string PrepaymentCode :: getStatus() {
    return status;
}

//사용됨으로 변경
void PrepaymentCode::setStatus(std::string& newStatus){
    if (newStatus == "Used")    status = newStatus;
}

