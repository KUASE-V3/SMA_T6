#include "service/OrderService.hpp"
#include "persistence/OrderRepository.hpp"
#include "persistence/DrinkRepository.hpp" // Drink 객체 사용 시 필요
#include "domain/order.h"              // domain::Order 직접 사용
#include "domain/drink.h"                // domain::Drink 직접 사용
#include "service/InventoryService.hpp"
#include "service/PrepaymentService.hpp"
#include "service/MessageService.hpp"    // 선결제 시 메시지 전송
#include "service/ErrorService.hpp"

#include <string>
#include <memory> // std::make_shared

namespace service {

OrderService::OrderService(
    persistence::OrderRepository& orderRepo,
    persistence::DrinkRepository& drinkRepo,
    service::InventoryService& inventoryService,
    service::PrepaymentService& prepaymentService,
    service::MessageService& messageService,
    service::ErrorService& errorService
) : orderRepository_(orderRepo),
    drinkRepository_(drinkRepo),
    inventoryService_(inventoryService),
    prepaymentService_(prepaymentService),
    messageService_(messageService),
    errorService_(errorService) {}

/**
 * @brief 사용자가 선택한 음료에 대한 주문을 생성합니다.
 * @param vmid 현재 자판기 ID.
 * @param selectedDrink 사용자가 선택한 음료의 상세 정보 (코드, 이름, 가격 포함).
 * @return 생성된 domain::Order 객체. 내부적으로 "PENDING" 상태로 저장됩니다.
 */
domain::Order OrderService::createOrder(const std::string& vmid, const domain::Drink& selectedDrink) {
    // UC2 (사용자 음료 선택 처리) 후, 재고 확인(UC3)을 거쳐 결제 요청(UC4) 전에 호출될 수 있음.
    // 사용자가 음료를 선택하면 Order를 생성 (상태: PENDING)
    try {
        // Order 객체 생성 (수량은 1로 고정, 인증코드는 아직 없음, 결제상태 PENDING)
        domain::Order newOrder(vmid, selectedDrink.getDrinkCode(), 1, "", "PENDING");
        orderRepository_.save(newOrder);
        return newOrder;
    } catch (const std::exception& e) {
        // 헤더 주석에는 VendingMachineException을 던진다고 되어 있으나, ErrorService 사용.
        errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "주문 생성 중 오류: " + std::string(e.what()));
        return domain::Order(); // 오류 시 기본 Order 객체 반환
    }
}

/**
 * @brief 주문에 대한 결제 승인 처리를 합니다.
 * @param order 처리할 주문 객체의 참조.
 * @param isPrepayment 해당 주문이 선결제인지 여부.
 */
void OrderService::processOrderApproval(domain::Order& order, bool isPrepayment) {
    // UC5 (결제 승인 처리)
    try {
        order.setPayStatus("APPROVED"); // UC5: "Order.Paymentstatus 를 APPROVED 로 설정" [cite: 12]
        orderRepository_.save(order); // 변경된 상태 저장

        if (!isPrepayment) {
            // 일반 결제: 재고 차감
            // UC7 (음료 배출 제어)의 전제조건은 결제 승인(UC5) [cite: 16]
            // UC7의 Typical Course 2: "일반결제인 경우 재고를 1감소시다" [cite: 16]
            inventoryService_.decreaseStock(order.getDrinkCode());
            // 이후 UserProcessController가 UC7(음료 배출)로 이동
        } else {
            // 선결제:
            // 1. 인증 코드 생성 (PrepaymentService 사용)
            // UC5: "...선결제인 경우 재고 확보 요청 (UC16) 단계를 실행한다." [cite: 12]
            // UC12 (인증코드 발급)이 UC5 이후에 호출됨
            std::string authCode = prepaymentService_.generateAuthCodeString();
            order.setCertCode(authCode); // 생성된 인증코드를 Order에 설정
            orderRepository_.save(order);    // 인증코드 포함하여 Order 업데이트

            // 2. PrepaymentService에 인증 코드와 Order 등록
            // Order를 shared_ptr로 전달해야 할 수 있으므로, Order 객체를 복사하거나 참조하는 방식 확인 필요.
            // 여기서는 Order 객체 자체가 PrepaymentCode에 저장될 필요는 없고, Order의 정보만 필요할 수 있음.
            // PrepaymentService::registerPrepayment는 std::shared_ptr<domain::Order>를 받음.
            // 현재 order는 참조이므로, 복사본을 만들어 shared_ptr로 전달하거나,
            // PrepaymentService가 Order의 주요 정보만 받도록 수정 필요.
            // 여기서는 임시로 Order의 복사본을 만들어 shared_ptr로 전달한다고 가정.
            auto orderCopyForPrepayment = std::make_shared<domain::Order>(order);
            prepaymentService_.registerPrepayment(authCode, orderCopyForPrepayment);

            // 3. MessageService를 통해 대상 자판기에 재고 확보 요청 (UC16)
            // 대상 자판기 정보는 UserProcessController가 알고 있거나, 이전에 결정되어야 함.
            // OrderService는 메시지 전송 자체보다는 MessageService를 호출하는 역할.
            // targetVm 정보가 필요함. 이 정보는 order 객체에 포함되거나, 별도로 전달받아야 함.
            // 여기서는 UserProcessController가 targetVm을 결정하고, 이후 MessageService를 호출한다고 가정.
            // 따라서, OrderService는 여기까지 처리하고, UserProcessController가 UC16 (메시지 전송)으로 흐름을 이어감.
            // UC5: "...결제 성공 메시지를 표시하고 UC7 또는 UC12-2 으로 이동한다." [cite: 12] (UC12-2는 인증코드 발급 후 화면 표시)
        }
    } catch (const std::exception& e) {
        // 이미 결제는 승인되었으므로, 이 단계에서 오류는 주로 내부 처리 오류(DB 등)
        order.setPayStatus("ERROR_POST_APPROVAL"); // 또는 다른 특정 오류 상태
        orderRepository_.save(order);
        errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "결제 승인 후 처리 중 오류: " + std::string(e.what()));
        // 이 경우 이미 결제된 금액에 대한 환불/보상 로직이 필요할 수 있으나, 현재 시스템 범위에는 없음.
    }
}

/**
 * @brief 주문에 대한 결제 거절 처리를 합니다.
 * @param order 처리할 주문 객체의 참조.
 */
void OrderService::processOrderDeclination(domain::Order& order) {
    // UC6 (결제 거절 처리)
    try {
        order.setPayStatus("DECLINED"); // UC6: "Order. Paymentstatus를 DECLINED 로 설정한다." [cite: 14]
        orderRepository_.save(order);
        // UC6: "사용자에게 결제 거절 타입에 따른 메시지를 표시한다." [cite: 14] (UserInterface 담당)
        // UC6: "음료 목록 조회 화면(UC1)으로 돌아간다." [cite: 14] (UserProcessController 담당)
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::UNEXPECTED_SYSTEM_ERROR, "결제 거절 처리 중 오류: " + std::string(e.what()));
    }
}

/**
 * @brief 특정 인증 코드로 주문을 조회합니다.
 * @param certCode 조회할 인증 코드.
 * @return 해당 인증 코드의 domain::Order 객체. 오류 시 기본 Order 객체.
 */
domain::Order OrderService::findOrderByCertCode(const std::string& certCode) {
    // 선결제된 음료를 다른 자판기에서 수령 시 사용 (UC13, UC14)
    try {
        domain::Order order = orderRepository_.findByCertCode(certCode);
        if (order.getCertCode().empty() || order.getCertCode() != certCode) { // findByCertCode가 못 찾거나 다른 코드 반환 시
            // 헤더 주석 ItemNotFoundException -> ErrorService 처리
            errorService_.processOccurredError(ErrorType::AUTH_CODE_NOT_FOUND, "인증 코드(" + certCode + ")에 해당하는 주문 없음");
            return domain::Order();
        }
        return order;
    } catch (const std::exception& e) {
        errorService_.processOccurredError(ErrorType::REPOSITORY_ACCESS_ERROR, "인증 코드로 주문 조회 중 오류: " + std::string(e.what()));
        return domain::Order();
    }
}

} // namespace service