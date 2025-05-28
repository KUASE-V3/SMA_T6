#ifndef PREPAY_CODE_REPOSITORY_H
#define PREPAY_CODE_REPOSITORY_H

#include <string>
#include <vector>

#include "domain/prepaymentCode.h" // domain::PrePaymentCode 사용

namespace persistence {

class PrepayCodeRepository {
public:
    PrepayCodeRepository() = default;

    // 인증코드로 선결제 정보 조회
    domain::PrePaymentCode findByCode(const std::string& code);
    // 선결제 정보 저장
    void save(const domain::PrePaymentCode& prepayCode);
    // 선결제 코드 상태 변경 (예: ACTIVE -> USED)
    bool updateStatus(const std::string& code, domain::CodeStatus newStatus); // domain::CodeStatus 명시
};

} // namespace persistence

#endif // PREPAY_CODE_REPOSITORY_H