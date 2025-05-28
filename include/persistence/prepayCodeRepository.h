#pragma once

#include <string>
#include <map>   

#include "domain/prepaymentCode.h"

namespace persistence {

/**
 * @brief 선결제 인증 코드(PrePaymentCode)의 영속성(저장, 조회, 상태 변경)을 관리하는 리포지토리 클래스입니다.
 * 내부적으로 std::map을 사용하여 메모리 내에 선결제 코드 정보를 저장합니다.
 * 관련된 유스케이스: UC12 (저장), UC14 (조회, 상태 변경), UC15 (저장)
 */
class PrepayCodeRepository {
public:
    /**
     * @brief PrepayCodeRepository 기본 생성자.
     */
    PrepayCodeRepository() = default;

    /**
     * @brief 주어진 인증 코드로 선결제 정보를 조회합니다. (UC14)
     * @param code 조회할 인증 코드 문자열.
     * @return 해당 코드를 가진 domain::PrePaymentCode 객체.
     * 찾지 못하면 코드가 비어있는 기본 생성된 PrePaymentCode 객체를 반환합니다.
     */
    domain::PrePaymentCode findByCode(const std::string& code);

    /**
     * @brief 새로운 선결제 정보(PrePaymentCode 객체)를 저장하거나 기존 정보를 업데이트합니다. (UC12, UC15)
     * PrePaymentCode 객체의 코드를 키로 사용하여 저장소에 추가하거나 덮어씁니다.
     * @param prepayCode 저장할 domain::PrePaymentCode 객체.
     */
    void save(const domain::PrePaymentCode& prepayCode);

    /**
     * @brief 특정 인증 코드의 상태를 변경합니다. (주로 ACTIVE -> USED, UC14)
     * @param code 상태를 변경할 인증 코드 문자열.
     * @param newStatus 변경할 새로운 domain::CodeStatus.
     * @return 상태 변경에 성공하면 true, 코드를 찾지 못하거나 변경할 수 없는 상태면 false.
     */
    bool updateStatus(const std::string& code, domain::CodeStatus newStatus);

private:
    // 메모리 내 선결제 코드 저장소. Key: 인증 코드(std::string), Value: PrePaymentCode 객체.
    std::map<std::string, domain::PrePaymentCode> codes_;
};

} // namespace persistence
