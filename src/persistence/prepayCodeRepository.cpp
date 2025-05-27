#include "persistence/prepayCodeRepository.h"
#include "domain/prepaymentCode.h"  // ��Ȯ�� ��� ��η� include


using namespace persistence;

std::vector<domain::PrepaymentCode> persistence::PrepaymentCodeRepository::prepaymentCodeList; 

void PrepaymentCodeRepository::save(const domain::PrepaymentCode& prepayCode) {

    prepaymentCodeList.push_back(prepayCode);
}



bool PrepaymentCodeRepository::isSameCode(const std::string& code) {
    for (const auto& pcode : prepaymentCodeList) {
        if (pcode.getCode() == code) {
            return true;
        }
    }
    return false;
}

void PrepaymentCodeRepository::changeStatus(const std::string& code) {
    for (auto& prepayCode : prepaymentCodeList) {
        if (prepayCode.getCode() == code) {
            prepayCode.use(code);  // PrepaymentCode의 use 메소드 호출
            break;
        }
    }
}
