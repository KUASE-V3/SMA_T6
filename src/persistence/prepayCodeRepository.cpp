#include "persistence/prepayCodeRepository.h"
#include "domain/prepaymentCode.h"  // ï¿½ï¿½È®ï¿½ï¿½ ï¿½ï¿½ï¿? ï¿½ï¿½Î·ï¿? include


using namespace persistence;

void PrepaymentCodeRepository::save(const domain::PrepaymentCode& prepayCode) {

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