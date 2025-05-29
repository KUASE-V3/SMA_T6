#ifndef PREPAYMENT_CODE_H
#define PREPAYMENT_CODE_H

#include <string>
#include <memory> // For std::shared_ptr

// Forward declaration (Order.h에 정의된 domain::Order를 가리킴)
namespace domain { class Order; }

namespace domain {

enum class CodeStatus { 
    ACTIVE,
    USED
};

class PrePaymentCode {
private:
    std::string code_attribute;
    CodeStatus status_attribute; // 이 enum도 domain::CodeStatus 또는 그냥 CodeStatus (같은 네임스페이스 내)
    std::shared_ptr<domain::Order> heldOrder_attribute; // domain::Order 명시

public:
    PrePaymentCode(std::string code = "",
                   CodeStatus status = CodeStatus::ACTIVE, // domain::CodeStatus 또는 CodeStatus
                   std::shared_ptr<domain::Order> order = nullptr) // domain::Order 명시
        : code_attribute(code), status_attribute(status), heldOrder_attribute(order) {}

    std::string getCode() const { return code_attribute; }
    CodeStatus getStatus() const { return status_attribute; } // domain::CodeStatus 또는 CodeStatus
    std::shared_ptr<domain::Order> getHeldOrder() const { return heldOrder_attribute; } // domain::Order 명시

    bool isUsable() const {
        return status_attribute == CodeStatus::ACTIVE; // CodeStatus::ACTIVE 또는 domain::CodeStatus::ACTIVE
    }

    void markAsUsed() {
        status_attribute = CodeStatus::USED; // CodeStatus::USED 또는 domain::CodeStatus::USED
    }

    void setHeldOrder(std::shared_ptr<domain::Order> order) { // domain::Order 명시
        heldOrder_attribute = order;
    }
};

} // namespace domain

#endif // PREPAYMENT_CODE_H