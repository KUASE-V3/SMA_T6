#pragma once

#include <string>
#include <memory> // std::shared_ptr 사용

#include "domain/order.h" 
#include "domain/drink.h" 

namespace persistence {
    class OrderRepository;
    class DrinkRepository;
}
namespace service {
    class ErrorService;
    class InventoryService;
    class PrepaymentService;
    class MessageService;
}

namespace service {

/**
 * @brief 주문 생성, 상태 변경(승인, 거절), 조회 등 주문 관련 비즈니스 로직을 처리하는 서비스입니다.
 * 관련된 유스케이스: UC2, UC3 (주문 생성 준비), UC5, UC6 (주문 상태 변경), UC14 (선결제 주문 조회)
 */
class OrderService {
public:
    /**
     * @brief OrderService 생성자.
     * @param orderRepo 주문 데이터 관리를 위한 OrderRepository 객체에 대한 참조.
     * @param drinkRepo 음료 기본 정보 조회를 위한 DrinkRepository 객체에 대한 참조.
     * @param inventoryService 재고 차감을 위한 InventoryService 객체에 대한 참조.
     * @param prepaymentService 선결제 처리를 위한 PrepaymentService 객체에 대한 참조.
     * @param messageService 메시지 전송 (현재 OrderService에서 직접 사용 안 함, UserProcessController가 사용).
     * @param errorService 오류 처리를 위한 ErrorService 객체에 대한 참조.
     */
    OrderService(
        persistence::OrderRepository& orderRepo,
        persistence::DrinkRepository& drinkRepo,
        service::InventoryService& inventoryService,
        service::PrepaymentService& prepaymentService,
        service::ErrorService& errorService
    );

    /**
     * @brief 사용자가 선택한 음료에 대한 새로운 주문을 생성합니다. (UC2, UC3 이후 호출)
     * 생성된 주문은 "PENDING" 상태로 저장됩니다.
     * @param vmid 주문이 발생한 현재 자판기의 ID.
     * @param selectedDrink 사용자가 선택한 음료의 domain::Drink 객체.
     * @return 생성된 domain::Order 객체. 오류 발생 시 기본 생성된 Order 객체 반환.
     */
    domain::Order createOrder(const std::string& vmid, const domain::Drink& selectedDrink);

    /**
     * @brief 주문에 대한 결제 승인 처리를 수행합니다. (UC5)
     * 주문 상태를 "APPROVED"로 변경하고, 일반 구매 시 재고를 차감합니다.
     * 선결제 주문 시에는 인증 코드를 생성하여 주문에 설정하고, PrepaymentService에 등록합니다.
     * @param order 처리할 주문 객체에 대한 참조 (이 함수 내에서 상태 및 정보가 변경될 수 있음).
     * @param isPrepayment 해당 주문이 선결제인지 여부.
     */
    void processOrderApproval(domain::Order& order, bool isPrepayment);

    /**
     * @brief 주문에 대한 결제 거절 처리를 수행합니다. (UC6)
     * 주문 상태를 "DECLINED"로 변경합니다.
     * @param order 처리할 주문 객체에 대한 참조.
     */
    void processOrderDeclination(domain::Order& order);

    /**
     * @brief 특정 인증 코드를 사용하여 선결제된 주문을 조회합니다. (UC14의 일부)
     * @param certCode 조회할 인증 코드.
     * @return 해당 인증 코드와 일치하는 domain::Order 객체.
     * 찾지 못하거나 오류 발생 시 기본 생성된 Order 객체 반환.
     */
    domain::Order findOrderByCertCode(const std::string& certCode);

private:
    persistence::OrderRepository& orderRepository_;     ///< 주문 데이터 접근용 리포지토리
    service::InventoryService& inventoryService_;         ///< 재고 관리용 서비스
    service::PrepaymentService& prepaymentService_;       ///< 선결제 처리용 서비스
    service::ErrorService& errorService_;                 ///< 오류 처리용 서비스
};

} // namespace service
