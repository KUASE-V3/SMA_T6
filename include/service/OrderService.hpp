#pragma once

#include <string>
#include <memory> // 전방 선언된 클래스의 포인터/참조를 멤버로 가질 때 필요할 수 있음

// Forward declarations
namespace domain {
    class Order;
    class Drink; // createOrder 시 Drink 상세 정보(예: 가격)를 사용하기 위함
}
namespace persistence {
    class OrderRepository;
    class DrinkRepository;    // 음료 가격 등 정보 조회용
}
namespace service {
    class ErrorService;
    class InventoryService;   // 일반 결제 시 재고 차감용
    class PrepaymentService;  // 선결제 시 인증 코드 발급 및 관련 처리용
    class MessageService;     // 선결제 시 다른 자판기에 메시지 전송용
    struct ErrorInfo;         // ErrorInfo를 반환하거나 사용할 경우
}

namespace service {

class OrderService {
public:
    OrderService(
        persistence::OrderRepository& orderRepo,
        persistence::DrinkRepository& drinkRepo,         // 음료 정보(가격) 조회
        service::InventoryService& inventoryService,     // 일반 구매 시 재고 차감
        service::PrepaymentService& prepaymentService,   // 선결제 처리
        service::MessageService& messageService,         // 선결제 시 메시지 전송
        service::ErrorService& errorService
    );
    // ~OrderService() = default; // 상속 고려 안 함

    /**
     * @brief 사용자가 선택한 음료에 대한 주문을 생성합니다.
     * @param vmid 현재 자판기 ID.
     * @param selectedDrink 사용자가 선택한 음료의 상세 정보 (코드, 이름, 가격 포함).
     * @return 생성된 domain::Order 객체. 내부적으로 "PENDING" 상태로 저장됩니다.
     * @throws VendingMachineException (또는 구체적인 예외) 생성 실패 시.
     */
    domain::Order createOrder(const std::string& vmid, const domain::Drink& selectedDrink);

    /**
     * @brief 주문에 대한 결제 승인 처리를 합니다.
     * 주문 상태를 "APPROVED"로 변경하고, 일반 구매인 경우 재고를 차감합니다.
     * 선결제 주문인 경우, PrepaymentService를 통해 인증 코드를 생성/등록하고,
     * MessageService를 통해 대상 자판기에 재고 확보 요청을 보냅니다.
     * @param order 처리할 주문 객체의 참조.
     * @param isPrepayment 해당 주문이 선결제인지 여부.
     * @return 모든 처리가 성공적으로 완료되면 true, 중간에 오류 발생 시 false (또는 예외).
     * 호출 측(UserProcessController)은 이 결과를 보고 다음 단계를 진행.
     * 여기서는 성공/실패를 ErrorInfo로 처리하도록 유도하기 위해 예외 발생 또는 ErrorInfo 반환 고려.
     * 일단 void로 두고, 내부에서 예외 발생 시 ErrorService를 통해 처리.
     */
    void processOrderApproval(domain::Order& order, bool isPrepayment);

    /**
     * @brief 주문에 대한 결제 거절 처리를 합니다.
     * 주문 상태를 "DECLINED"로 변경합니다.
     * @param order 처리할 주문 객체의 참조.
     */
    void processOrderDeclination(domain::Order& order);

    /**
     * @brief 특정 인증 코드로 주문을 조회합니다.
     * @param certCode 조회할 인증 코드.
     * @return 해당 인증 코드의 domain::Order 객체.
     * @throws ItemNotFoundException (또는 ErrorInfo 통해 처리) 코드가 유효하지 않거나 주문을 찾을 수 없는 경우.
     */
    domain::Order findOrderByCertCode(const std::string& certCode);


private:
    persistence::OrderRepository& orderRepository_;
    persistence::DrinkRepository& drinkRepository_;
    service::InventoryService& inventoryService_;
    service::PrepaymentService& prepaymentService_;
    service::MessageService& messageService_;
    service::ErrorService& errorService_;

    // 내부 헬퍼 함수
    // void deductStockForStandardPurchase(const domain::Order& order);
    // void handlePrepaymentLogic(const domain::Order& order);
};

} // namespace service