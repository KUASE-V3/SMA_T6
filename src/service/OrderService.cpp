#include "service/OrderService.hpp"
#include "persistence/OrderRepository.hpp"    
#include "persistence/DrinkRepository.hpp"  
#include "domain/order.h"                 
#include "domain/drink.h"                 
#include "service/InventoryService.hpp"
#include "service/PrepaymentService.hpp"
#include "service/ErrorService.hpp"

#include <string>
#include <memory> // std::make_shared
#include <stdexcept> 

namespace service {

OrderService::OrderService(
    persistence::OrderRepository& orderRepo,
    service::InventoryService& inventoryService,
    service::PrepaymentService& prepaymentService,
    service::ErrorService& errorService
) : orderRepository_(orderRepo),
    inventoryService_(inventoryService),
    prepaymentService_(prepaymentService),
    errorService_(errorService) {}

domain::Order OrderService::createOrder(const std::string& vmid, const domain::Drink& selectedDrink) {
    try {
        // 우리 자판기의 주문의 음료 수량은 1개로 정함
        domain::Order newOrder(vmid, selectedDrink.getDrinkCode(), 1, "", "PENDING");
        orderRepository_.save(newOrder);
        return newOrder;
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "주문 생성 중 저장소 오류: " + std::string(e.what()));
        return domain::Order(); // 빈 Order 객체 반환
    }
}
void OrderService::processOrderApproval(domain::Order& order, bool isPrepayment) {
    // UC5 - Typical Course 2: Order.Paymentstatus 를 APPROVED 로 설정
    try {
        order.setPayStatus("APPROVED");

        if (isPrepayment) {
            // 선결제인 경우: 인증 코드 생성 및 주문에 설정 (UC12의 일부)
            std::string authCode = prepaymentService_.generateAuthCodeString(); // PFR R4.2
            order.setCertCode(authCode);
            // 인증 코드 포함하여 주문 정보 업데이트
            orderRepository_.save(order);

        
            // UC5 -> UC16 (재고 확보 요청)은 UserProcessController가 MessageService를 통해 진행.
        } else {
            // 일반 구매인 경우: 주문 정보 업데이트 (주로 상태 변경) 후 재고 차감
            orderRepository_.save(order);
            // UC7 - Typical Course 2: 일반결제인 경우 재고를 1 감소
            inventoryService_.decreaseStock(order.getDrinkCode());
        }
        // UC5 - Typical Course 3: 결제 성공 메시지 표시 및 다음 단계 이동은 UserProcessController가 담당.
    } catch (const std::exception& e) {
        order.setPayStatus("ERROR_POST_APPROVAL"); // 오류 발생 시 주문 상태 변경
        try {
            orderRepository_.save(order);
        } catch (...) {  }
        errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "결제 승인 후 내부 처리 오류: " + std::string(e.what()));
    }
}

void OrderService::processOrderDeclination(domain::Order& order) {
    // UC6 - Typical Course 2: Order.Paymentstatus를 DECLINED 로 설정한다.
    try {
        order.setPayStatus("DECLINED");
        orderRepository_.save(order);
        // UC6 - Typical Course 3 & 4: 사용자 메시지 표시 및 UC1로 이동은 UserProcessController가 담당.
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "결제 거절 처리 중 시스템 오류: " + std::string(e.what()));
    }
}

domain::Order OrderService::findOrderByCertCode(const std::string& certCode) {
    // UC14 - Typical Course 1: 저장하고 있는 코드와 입력 코드가 일치하는지 확인 (OrderRepository가 담당)
    try {
        domain::Order order = orderRepository_.findByCertCode(certCode);
        if (order.getCertCode().empty() || order.getCertCode() != certCode) {
            // OrderRepository::findByCertCode가 못 찾으면 기본 Order 객체 반환 가정
            // UC14 E1: 시스템에 저장되어 있지 않은 인증코드
            errorService_.processOccurredError(ErrorType::AUTH_CODE_NOT_FOUND, "인증 코드(" + certCode + ")에 해당하는 주문을 찾을 수 없습니다.");
            return domain::Order(); // 빈 객체 반환
        }
        return order;
    } catch (const std::exception& e) { // Repository 접근 등 예외
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "인증 코드로 주문 조회 중 시스템 오류: " + std::string(e.what()));
        return domain::Order();
    }
}

} // namespace service
