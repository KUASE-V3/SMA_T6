#ifndef PREPAYMENTCODEREPOSITORY_H
#define PREPAYMENTCODEREPOSITORY_H

#include <vector>
#include <string>
#include "domain/order.h"
#include "domain/prepaymentCode.h"


namespace persistence {

class PrepaymentCodeRepository {
public:
    static void save(const domain::PrepaymentCode& code);
    static bool isSameCode(const std::string& code);
    static void changeStatus(const std::string& code);

private:
    static std::vector<domain::PrepaymentCode> prepaymentCodeList; // ���� ĳ��
};
}

#endif