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

class OrderService {
public:

    OrderService(
        persistence::OrderRepository& orderRepo,
        service::InventoryService& inventoryService,
        service::PrepaymentService& prepaymentService,
        service::ErrorService& errorService
    );

    domain::Order createOrder(const std::string& vmid, const domain::Drink& selectedDrink);
    void processOrderApproval(domain::Order& order, bool isPrepayment);
    void processOrderDeclination(domain::Order& order);
    domain::Order findOrderByCertCode(const std::string& certCode);

private:
    persistence::OrderRepository& orderRepository_;     ///< 주문 데이터 접근용 리포지토리
    service::InventoryService& inventoryService_;         ///< 재고 관리용 서비스
    service::PrepaymentService& prepaymentService_;       ///< 선결제 처리용 서비스
    service::ErrorService& errorService_;                 ///< 오류 처리용 서비스
};

} // namespace service
