#pragma once

#include <string>
#include <vector>
#include <utility> 

#include "domain/drink.h" // domain::Drink 객체 사용

namespace persistence {
    class InventoryRepository;
    class DrinkRepository;
}
namespace service {
    class ErrorService; // ErrorService 객체 사용
}

namespace service {

/**
 * @brief 자판기의 재고 관련 비즈니스 로직을 처리하는 서비스입니다.
 * 음료 목록 조회, 특정 음료의 재고 및 가격 확인, 재고 차감 등의 기능을 제공합니다.
 * 관련된 유스케이스: UC1, UC2, UC3, UC7, UC15, UC17
 */
class InventoryService {
public:
    /**
     * @brief InventoryService 생성자.
     * @param inventoryRepo 재고 데이터 관리를 위한 InventoryRepository 객체에 대한 참조.
     * @param drinkRepo 음료 기본 정보 관리를 위한 DrinkRepository 객체에 대한 참조.
     * @param errorService 오류 처리를 위한 ErrorService 객체에 대한 참조.
     */
    InventoryService(
        persistence::InventoryRepository& inventoryRepo,
        persistence::DrinkRepository& drinkRepo,
        service::ErrorService& errorService
    );

    /**
     * @brief 시스템에 정의된 모든 음료 종류의 목록을 반환합니다. (UC1)
     * @return 모든 음료의 domain::Drink 객체를 담은 벡터.
     * 오류 발생 시 빈 벡터를 반환하고 ErrorService를 통해 오류를 보고합니다.
     */
    std::vector<domain::Drink> getAllDrinkTypes();

    /**
     * @brief 특정 음료의 현재 자판기 내 판매 가능 여부, 가격, 실제 재고량을 담는 구조체.
     */
    struct DrinkAvailabilityInfo {
        bool isAvailable = false;   ///< 현재 자판기에서 구매 가능한지 (취급하며 재고도 0 초과인지)
        int price = 0;              ///< 음료 가격
        int currentStock = 0;       ///< 현재 자판기의 해당 음료 실제 재고량
    };

    /**
     * @brief 특정 음료의 현재 자판기 내 판매 가능 여부, 가격, 실제 재고량을 확인합니다. (UC2, UC3)
     * @param drinkCode 확인할 음료의 코드.
     * @return DrinkAvailabilityInfo 구조체. 오류 발생 시 isAvailable=false, price=0, currentStock=0으로 반환될 수 있음.
     */
    DrinkAvailabilityInfo checkDrinkAvailabilityAndPrice(const std::string& drinkCode);

    /**
     * @brief 특정 음료의 재고를 1개 감소시킵니다. (주로 일반 구매 시 사용 - UC7)
     * @param drinkCode 재고를 감소시킬 음료의 코드.
     * 오류(재고 부족 등) 발생 시 ErrorService를 통해 보고합니다.
     */
    void decreaseStock(const std::string& drinkCode);

    /**
     * @brief 특정 음료의 재고를 지정된 수량만큼 감소시킵니다. (주로 외부 선결제 요청 처리 시 사용 - UC15)
     * @param drinkCode 재고를 감소시킬 음료의 코드.
     * @param amount 감소시킬 수량.
     * 오류(재고 부족 등) 발생 시 ErrorService를 통해 보고합니다.
     */
    void decreaseStockByAmount(const std::string& drinkCode, int amount);

private:
    persistence::InventoryRepository& inventoryRepository_; ///< 재고 데이터 접근용 리포지토리
    persistence::DrinkRepository& drinkRepository_;       ///< 음료 기본 정보 접근용 리포지토리
    service::ErrorService& errorService_;                 ///< 오류 처리용 서비스
};

} // namespace service
