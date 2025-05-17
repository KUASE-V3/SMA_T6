#include "prepayCodeRepository.h"

void PrepaymentCodeRepository::save(const PrepaymentCode& prepayCode) {
    prepaymentCodeList.push_back(prepayCode);
}



bool PrepaymentCodeRepository::isSameCode(const std::string& code) const {
    for (const auto& pcode : prepaymentCodeList) {
        if (pcode.getCode() == code) {
            return true;
        }
    }
    return false;
}