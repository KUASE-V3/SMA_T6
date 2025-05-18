#ifndef PREPAYMENTCODEREPOSITORY_H
#define PREPAYMENTCODEREPOSITORY_H

#include <vector>
#include <string>
#include "domain/order.h"
#include "domain/prepaymentCode.h"


namespace persistence {

class PrepaymentCodeRepository {
public:
    void save(const domain::PrepaymentCode& code);
    bool isSameCode(const std::string& code) const;

private:
    std::vector<domain::PrepaymentCode> prepaymentCodeList; // ���� ĳ��
};
}

#endif