#pragma once

#include <string>
#include <memory>
#include <optional> // 유효성 검증 결과 반환 시 사용 가능

// Forward declarations
namespace domain {
    class Order;
    class PrePaymentCode;
    // enum class CodeStatus; // PrePaymentCode.h 에 함께 정의되어 있음
}
namespace persistence {
    class PrepayCodeRepository;
    class OrderRepository; // recordIncomingPrepayment 시 Order 생성/저장 위해 필요
}
namespace service {
    class ErrorService;
    // struct ErrorInfo;       // ErrorService.h 에 정의되어 있음
    // enum class ErrorType;   // ErrorService.h 에 정의되어 있음
}

namespace service {



class PrepaymentService {
public:
    PrepaymentService(
        persistence::PrepayCodeRepository& prepayCodeRepo,
        persistence::OrderRepository& orderRepo, // recordIncomingPrepayment 및 PrePaymentCode와 Order 연결 위해
        service::ErrorService& errorService
    );
    // ~PrepaymentService() = default; // 상속 고려 안 함

    /**
     * @brief 랜덤한 5자리 영숫자 인증 코드 문자열을 생성하여 반환합니다.
     * @return 생성된 인증 코드 문자열.
     */
    std::string generateAuthCodeString();

    /**
     * @brief 생성된 인증 코드 문자열과 관련 Order를 받아 PrePaymentCode 객체를 생성하고 저장합니다.
     * @param authCode generateAuthCodeString()으로 생성된 인증 코드.
     * @param order 이 선결제와 연결될 주문 객체의 shared_ptr.
     * @throws VendingMachineException (또는 구체적 예외) 저장 실패 시.
     */
    void registerPrepayment(const std::string& authCode, std::shared_ptr<domain::Order> order);

    /**
     * @brief 입력된 인증 코드의 형식이 유효한지 (예: 5자리 영숫자) 검사합니다.
     * @param authCode 검사할 인증 코드 문자열.
     * @return 형식이 유효하면 true, 아니면 false.
     */
    bool isValidAuthCodeFormat(const std::string& authCode) const;

    /**
     * @brief 입력된 인증 코드가 저장되어 있고 사용 가능한지(ACTIVE) 확인합니다.
     * 유효하고 사용 가능하면, 연결된 주문의 주요 정보(예: 음료 코드)를 포함한 PrePaymentCode 객체를 반환합니다.
     * @param authCode 검증할 인증 코드 문자열.
     * @return 유효하고 사용 가능한 경우 해당 domain::PrePaymentCode 객체.
     * @throws CodeNotFoundException 코드가 존재하지 않는 경우.
     * @throws CodeAlreadyUsedException 코드가 이미 사용된 경우.
     * @throws InvalidCodeException 기타 유효하지 않은 코드일 경우.
     */
    domain::PrePaymentCode getPrepaymentDetailsIfActive(const std::string& authCode);

    /**
     * @brief 인증 코드의 상태를 "USED"로 변경합니다. (시퀀스 다이어그램의 ChangeStatusCode)
     * 음료 제공 절차가 성공적으로 완료된 후 호출됩니다.
     * @param authCode 사용 완료 처리할 인증 코드 문자열.
     * @throws CodeNotFoundException 해당 코드를 찾을 수 없는 경우.
     * @throws VendingMachineException (또는 구체적 예외) 상태 변경 실패 시.
     */
    void changeAuthCodeStatusToUsed(const std::string& authCode);

    /**
     * @brief 다른 자판기로부터 수신한 선결제 요청 정보를 등록합니다.
     * 수신한 인증 코드, 음료 코드 등의 정보로 PrePaymentCode 객체를 생성/저장하고,
     * 이와 연결될 (최소한의 정보를 가진) Order 객체도 생성/저장합니다.
     * @param certCode 수신한 인증 코드.
     * @param drinkCode 수신한 음료 코드.
     * @param vmidForOrder 이 선결제 건을 위해 생성될 Order에 기록될 자판기 ID (보통 요청한 자판기 ID).
     * @throws VendingMachineException (또는 구체적 예외) 등록 실패 시.
     */
    void recordIncomingPrepayment(const std::string& certCode, const std::string& drinkCode, const std::string& vmidForOrder);


private:
    persistence::PrepayCodeRepository& prepayCodeRepository_;
    persistence::OrderRepository& orderRepository_; // Order 생성 및 저장 위해 추가
    service::ErrorService& errorService_;

    std::string generateRandomAlphanumericString(size_t length) const; // generateAuthCodeString의 실제 구현
};

} // namespace service