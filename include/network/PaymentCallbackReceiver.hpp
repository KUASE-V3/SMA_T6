#pragma once

#include <functional> 
#include <thread>     
#include <chrono>     
#include <random>    
namespace network {

/**
 * @brief 외부 결제 시스템의 응답을 시뮬레이션하는 클래스입니다.
 * 실제 카드사 연동 대신, 일정 시간 지연 후 확률적으로 성공 또는 실패를 반환합니다.
 * UC4 (사용자 결제 요청)에서 카드 시스템 응답을 모방하는 데 사용됩니다.
 */
class PaymentCallbackReceiver {
public:
    /**
     * @brief 결제 시뮬레이션 완료 시 호출될 콜백 함수의 타입 정의입니다.
     * @param success 결제 시뮬레이션 성공 여부 (true: 성공, false: 실패).
     */
    using Callback = std::function<void(bool success)>;

    /**
     * @brief PaymentCallbackReceiver 기본 생성자.
     */
    PaymentCallbackReceiver() = default;

    /**
     * @brief 선결제(또는 일반 결제) 과정을 시뮬레이션합니다.
     * 지정된 시간(delay_seconds)만큼 지연 후, 확률적으로 성공 또는 실패 결과를 콜백 함수를 통해 전달합니다.
     * @param cb 결과(성공/실패)를 전달받을 콜백 함수.
     * @param delay_seconds (선택적) 결제 처리 시간을 시뮬레이션하기 위한 지연 시간(초). 기본값은 3초.
     */
    void simulatePrepayment(Callback cb, int delay_seconds = 3) const;
};

} // namespace network
