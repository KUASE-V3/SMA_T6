#include "prepaymentCode.h"
#include <cstdlib>     // rand, srand
#include <ctime>       // time
#include <sstream>     // stringstream
#include<string>
#include <cctype> // std::isalnum, std::isupper


namespace domain {
PrepaymentCode::PrepaymentCode() {
    code = rand();         // 5?ë¦? ?œ?¤ ì½”ë“œ ?ƒ?„±
    status = "Unused";     // ê¸°ë³¸ ?ƒ?ƒœ
    
}

// ?œ?¤ 5?ë¦? ë¬¸ì?—´ ?ƒ?„± ?•¨?ˆ˜
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


//?¸ì¦ì½”?“œ ?˜•?‹ ê²??‚¬
bool PrepaymentCode::isUsable( std::string& code) {
    if (code.length() != 5) return false;

    for (char c : code) {
        if (!std::isalnum(c)) return false;               // ?ˆ«??‚˜ ë¬¸ì ?•„?‹Œê²½ìš°,
        if (std::isalpha(c) && !std::isupper(c)) return false;      // ?•Œ?ŒŒë²³ì¼ ?•Œ, ?†Œë¬¸ì?¼ ê²½ìš° false
    }

    return true;
}


// ?“¤?–´?˜¨ ì£¼ë¬¸ê³? ?—°ê²?
PrepaymentCode PrepaymentCode::hold(Order order) {
    heldOrder = order;
    return *this;
}




//?‚¬?š©?¨?œ¼ë¡? ë³?ê²?
void PrepaymentCode::setStatus(std::string& newStatus){
    if (newStatus == "Used")    status = newStatus;
}



std::string PrepaymentCode :: getCode() const {
    return code;
}

}