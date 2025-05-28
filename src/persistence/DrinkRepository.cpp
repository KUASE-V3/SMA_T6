#include "persistence/DrinkRepository.hpp"
#include "domain/drink.h" 
#include <vector>
#include <stdexcept> // std::logic_error 사용을 위해 포함


namespace persistence {

// 20종의 음료 데이터를 저장하는 정적 상수 벡터
namespace {
    const std::vector<domain::Drink> hardcoded_drinks = {
        // 가격 1000원 기준
        domain::Drink("01", "콜라", 1000),
        domain::Drink("02", "사이다", 1000),
        domain::Drink("03", "녹차", 1000),
        domain::Drink("04", "홍차", 1000),
        domain::Drink("05", "밀크티", 1000),
        domain::Drink("06", "탄산수", 1000),
        domain::Drink("07", "보리차", 1000),
        domain::Drink("08", "캔커피", 1000),
        domain::Drink("09", "물", 1000),
        domain::Drink("10", "에너지드링크", 1000),
        domain::Drink("11", "유자차", 1000),
        domain::Drink("12", "식혜", 1000),
        domain::Drink("13", "아이스티", 1000),
        domain::Drink("14", "딸기주스", 1000),
        domain::Drink("15", "오렌지주스", 1000),
        domain::Drink("16", "포도주스", 1000),
        domain::Drink("17", "이온음료", 1000),
        domain::Drink("18", "아메리카노", 1000),
        domain::Drink("19", "핫초코", 1000),
        domain::Drink("20", "카페라떼", 1000)
    };
} 


// findByDrinkCode 메소드 구현
domain::Drink DrinkRepository::findByDrinkCode(const std::string& drinkCode) {
    for (const auto& drink : hardcoded_drinks) {
        if (drink.getDrinkCode() == drinkCode) {
            return drink;
        }
    }
    // drinkCode가 유효하지 않은 경우 예외 처리
}
// findAll 메소드 구현
std::vector<domain::Drink> DrinkRepository::findAll() {
    return hardcoded_drinks; // 하드코딩된 음료 목록 전체를 반환
}

} // namespace persistence
