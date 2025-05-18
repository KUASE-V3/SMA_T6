#ifndef PREPAYMENTCODEREPOSITORY_H
#define PREPAYMENTCODEREPOSITORY_H

#include <vector>
#include <string>
#include "../include/domain/order.h"
#include "../include/domain/prepaymentCode.h"

class PrepaymentCodeRepository {
public:
    void save(const PrepaymentCode& prepayCode);
    bool isSameCode(const std::string& code) const;

private:
    std::vector<PrepaymentCode> prepaymentCodeList;
};

#endif