#pragma once

#include <string>
#include <memory>   // std::shared_ptr

#include "domain/order.h"          
#include "domain/prepaymentCode.h" 

namespace persistence {
    class PrepayCodeRepository;
    class OrderRepository;
}
namespace service {
    class ErrorService; // ErrorService 객체 사용
}

namespace service {

/**
 * @brief 선결제 관련 비즈니스 로직을 처리하는 서비스입니다.
 * 인증 코드 생성, 등록, 유효성 검사, 상태 변경 및 다른 자판기로부터의 선결제 요청 기록 등을 담당합니다.
 * 관련된 유스케이스: UC12, UC13, UC14, UC15
 */
class PrepaymentService {
public:
    /**
     * @brief PrepaymentService 생성자.
     * @param prepayCodeRepo 선결제 코드 데이터 관리를 위한 PrepayCodeRepository 객체에 대한 참조.
     * @param orderRepo 주문 데이터 관리를 위한 OrderRepository 객체에 대한 참조 (선결제 시 주문 기록용).
     * @param errorService 오류 처리를 위한 ErrorService 객체에 대한 참조.
     */
    PrepaymentService(
        persistence::PrepayCodeRepository& prepayCodeRepo,
        persistence::OrderRepository& orderRepo,
        service::ErrorService& errorService
    );

    /**
     * @brief 랜덤한 5자리 영숫자 인증 코드 문자열을 생성하여 반환합니다. (UC12)
     * (PFR R4.2: 사용자가 선결제를 요청할 때마다 시스템은 새로운 인증코드를 생성해야 한다.)
     * @return 생성된 5자리 인증 코드 문자열.
     */
    std::string generateAuthCodeString();



    /**
     * @brief 입력된 인증 코드의 형식이 유효한지 (5자리 영숫자) 검사합니다. (UC13)
     * (UC13 Typical Course 3: 입력 문자열이 정규식 “[A-Za-z0-9]{5}”와 일치하는지 검사한다.)
     * @param authCode 검사할 인증 코드 문자열.
     * @return 형식이 유효하면 true, 아니면 false.
     */
    bool isValidAuthCodeFormat(const std::string& authCode) const;

    /**
     * @brief 입력된 인증 코드가 저장되어 있고 사용 가능한지(ACTIVE 상태) 확인합니다. (UC14)
     * 유효하고 사용 가능하면, 해당 인증 코드에 연결된 주문 정보를 포함한 PrePaymentCode 객체를 반환합니다.
     * (UC14 Typical Course 1: 저장하고 있는 코드와 입력 코드가 일치하는지 확인한다.)
     * @param authCode 검증할 인증 코드 문자열.
     * @return 유효하고 사용 가능한 경우 해당 domain::PrePaymentCode 객체.
     * 그렇지 않거나 오류 발생 시 기본 생성된(코드가 비어있는) PrePaymentCode 객체 반환.
     */
    domain::PrePaymentCode getPrepaymentDetailsIfActive(const std::string& authCode);

    /**
     * @brief 인증 코드의 상태를 "USED"로 변경합니다. (UC14)
     * 음료 제공 절차가 성공적으로 완료된 후 호출됩니다.
     * (UC14 Typical Course 2: 일치할 경우 AuthCode를 만료시킨다.)
     * @param authCode 사용 완료 처리할 인증 코드 문자열.
     * 오류 발생 시 ErrorService를 통해 보고합니다.
     */
    void changeAuthCodeStatusToUsed(const std::string& authCode);

    /**
     * @brief 다른 자판기로부터 수신한 선결제 요청 정보를 바탕으로,
     * PrePaymentCode 객체와 관련 Order 객체를 생성하고 저장합니다. (UC15)
     * (UC15 Typical Course 2: ... PrePayment.code = AuthCode.code로 저장한다)
     * @param certCode 수신한 인증 코드.
     * @param drinkCode 수신한 음료 코드.
     * @param vmidForOrder 이 선결제 건을 위해 생성될 Order에 기록될 자판기 ID (요청을 보낸 자판기의 ID).
     * 오류 발생 시 ErrorService를 통해 보고합니다.
     */
    void recordIncomingPrepayment(const std::string& certCode, const std::string& drinkCode, const std::string& vmidForOrder);

private:
    persistence::PrepayCodeRepository& prepayCodeRepository_; ///< 선결제 코드 데이터 접근용 리포지토리
    persistence::OrderRepository& orderRepository_;         ///< 주문 데이터 접근용 리포지토리 (선결제 시 주문 기록)
    service::ErrorService& errorService_;                   ///< 오류 처리용 서비스

    /**
     * @brief 지정된 길이의 랜덤 영숫자 문자열을 생성하는 내부 헬퍼 함수입니다.
     * @param length 생성할 문자열의 길이.
     * @return 생성된 랜덤 영숫자 문자열.
     */
    std::string generateRandomAlphanumericString(size_t length) const;
};

} // namespace service
