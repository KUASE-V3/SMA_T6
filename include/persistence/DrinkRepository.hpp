#pragma once

#include <string>
#include <vector>

#include "domain/drink.h" // Drink 클래스 사용

namespace persistence {

class DrinkRepository {
public:
    DrinkRepository() = default; // 기본 생성자 (필요시)

    // 특정 음료 코드로 음료 정보를 조회
    // 현재 오류 처리 범위 정책에 따라, 항상 유효한 drinkCode로 호출된다고 가정
    domain::Drink findByDrinkCode(const std::string& drinkCode);

    // 시스템에 정의된 모든 음료 종류의 목록을 반환
    std::vector<domain::Drink> findAll();
};

} // namespace persistence