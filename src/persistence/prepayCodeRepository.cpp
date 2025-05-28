#include "persistence/prepayCodeRepository.h"
#include <map> // 내부 저장소로 std::map 사용

namespace persistence {


// 인증코드로 선결제 정보 조회
domain::PrePaymentCode PrepayCodeRepository::findByCode(const std::string& code) {
    auto it = codes_.find(code);
    if (it != codes_.end()) {
        return it->second;
    }
    // 코드를 찾지 못한 경우, 기본 생성된 PrePaymentCode 객체를 반환합니다.
    // UC14 (인증코드 유효성 검증)의 예외 상황 E1: "시스템에 저장되 있지 않은 인증코드이면, 시스템은 에러메시지를 호출해 보여준다." [cite: 30]
    // 이 리포지토리 메소드는 찾는 역할만 하며, 실제 에러 메시지 표시는 서비스 계층이나 컨트롤러에서 처리합니다.
    return domain::PrePaymentCode(); // 기본 객체 반환
}

// 선결제 정보 저장
void PrepayCodeRepository::save(const domain::PrePaymentCode& prepayCode) {
    // prepayCode의 code_attribute를 키로 사용하여 정보를 저장하거나 업데이트합니다.
    // PrePaymentCode 객체 자체가 코드 문자열을 가지고 있으므로, prepayCode.getCode()를 사용합니다.
    codes_[prepayCode.getCode()] = prepayCode;
    // UC12 (인증코드 발급) 단계에서 생성된 인증코드가 저장됩니다. [cite: 25]
    // UC15 (선결제요청 수신 및 재고 확보) 단계에서도 다른 자판기의 요청으로 코드가 저장될 수 있습니다. [cite: 32]
    // 이때 PrePayment.code = AuthCode.code로 저장하는 로직이 있음. [cite: 32]
}

// 선결제 코드 상태 변경 (예: ACTIVE -> USED)
bool PrepayCodeRepository::updateStatus(const std::string& code, domain::CodeStatus newStatus) {
    auto it = codes_.find(code);
    if (it != codes_.end()) {

        if (newStatus == domain::CodeStatus::USED) {
            if (it->second.isUsable()) { // ACTIVE 상태일 때만 USED로 변경 가능
                it->second.markAsUsed(); // UC14: "일치할 경우 AuthCode를 만료시킨다." [cite: 30] (만료를 USED 상태로 해석)
                return true;
            } else {
                // 이미 USED 상태이거나 다른 비활성 상태일 수 있음
                return false; 
            }
        }
        return false; // 그 외 상태 변경은 미지원
    }
    return false; // 코드를 찾지 못함
}

} // namespace persistence