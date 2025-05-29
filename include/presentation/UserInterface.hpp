#pragma once

#include <string>
#include <vector>
#include "domain/drink.h"
#include "domain/inventory.h"
#include "domain/vendingMachine.h"

namespace presentation {

/**
 * @brief 사용자 인터페이스(CLI 기반)를 담당하는 클래스입니다.
 * 사용자로부터 입력을 받고, 시스템의 정보를 화면에 표시합니다.
 */
class UserInterface {
public:
    UserInterface() = default;

    // --- 메뉴 및 일반 입력 관련 ---
    /**
     * @brief 메인 메뉴 옵션들을 화면에 표시합니다.
     * @param options 표시할 메뉴 옵션 문자열 벡터.
     */
    void displayMainMenu(const std::vector<std::string>& options);

    /**
     * @brief 사용자로부터 메뉴 선택(숫자)을 입력받고 유효성을 검사합니다.
     * @param maxChoice 선택 가능한 최대 메뉴 번호.
     * @return 사용자가 선택한 유효한 메뉴 번호.
     */
    int getUserChoice(int maxChoice);

    /**
     * @brief 사용자로부터 일반 문자열 입력을 받습니다.
     * @param prompt 사용자에게 보여줄 안내 메시지.
     * @return 사용자가 입력한 문자열.
     */
    std::string getUserInputString(const std::string& prompt);

    // --- 음료 정보 표시 및 선택 ---
    /**
     * @brief 모든 음료 목록을 화면에 표시합니다. (UC1)
     * @param allDrinks 표시할 전체 음료 목록 (domain::Drink 객체의 벡터).
     * @param currentVmInventory 현재 자판기의 재고 정보 (이번 과제에서는 음료 목록 표시에 직접 사용되지 않을 수 있음).
     */
    void displayDrinkList(const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory);

    /**
     * @brief 사용자로부터 구매할 음료의 코드를 입력받습니다. (UC2)
     * @param allDrinks 선택 가능한 전체 음료 목록 (코드 유효성 검사용).
     * @param currentVmInventory 현재 자판기 재고 (참고용, 직접 사용 안 함).
     * @return 사용자가 입력한 음료 코드 (유효하거나, 취소 시 빈 문자열).
     */
    std::string selectDrink(const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory);

    // --- 결제 관련 UI ---
    /**
     * @brief 결제할 금액과 함께 카드 삽입을 요청하는 메시지를 표시합니다. (UC4)
     * @param amount 결제할 금액.
     */
    void displayPaymentPrompt(int amount);

    /**
     * @brief 사용자에게 결제를 계속 진행할지 여부를 확인받습니다. (Y/N)
     * @return 사용자가 동의하면 true, 그렇지 않으면 false.
     */
    bool confirmPayment();

    /**
     * @brief "결제 진행 중" 메시지를 표시합니다. (UC4)
     */
    void displayPaymentProcessing();

    /**
     * @brief 결제 결과를 사용자에게 표시합니다. (UC5, UC6)
     * @param success 결제 성공 여부.
     * @param message 결과와 함께 표시할 추가 메시지.
     */
    void displayPaymentResult(bool success, const std::string& message);

    // --- 선결제 관련 UI ---
    /**
     * @brief 특정 음료에 대해 다른 자판기에서 선결제할지 여부를 사용자에게 확인받습니다. (UC11)
     * @param drinkName 선결제 대상 음료의 이름.
     * @return 사용자가 동의하면 true, 그렇지 않으면 false.
     */
    bool confirmPrepayment(const std::string& drinkName);

    /**
     * @brief 발급된 인증 코드와 수령 자판기 정보를 사용자에게 안내합니다. (UC12)
     * @param authCode 발급된 인증 코드.
     * @param targetVm 음료를 수령할 대상 자판기 정보.
     * @param drinkName 선결제한 음료의 이름.
     */
    void displayAuthCode(const std::string& authCode, const domain::VendingMachine& targetVm, const std::string& drinkName);

    /**
     * @brief 사용자로부터 인증 코드를 입력받습니다. (UC13)
     * @return 사용자가 입력한 인증 코드 (형식 검사는 서비스 계층에서 수행). 취소 시 빈 문자열.
     */
    std::string getAuthCodeInput();

    // --- 자판기 및 재고 관련 상태 메시지 ---
    /**
     * @brief 선택한 음료가 현재 자판기에 재고 없음을 알립니다. (UC3)
     * @param drinkName 재고 없는 음료의 이름.
     */
    void displayOutOfStockMessage(const std::string& drinkName);

    /**
     * @brief 구매 가능한 가장 가까운 다른 자판기의 정보를 안내합니다. (UC10)
     * @param vm 가장 가까운 자판기 정보.
     * @param drinkName 찾고 있는 음료의 이름.
     */
    void displayNearestVendingMachine(const domain::VendingMachine& vm, const std::string& drinkName);

    /**
     * @brief 주변 다른 자판기에서 해당 음료의 재고를 찾을 수 없음을 알립니다. (UC9 타임아웃 등)
     * @param drinkName 찾고 있던 음료의 이름.
     */
    void displayNoOtherVendingMachineFound(const std::string& drinkName);

    // --- 일반 메시지 및 오류 메시지 ---
    /**
     * @brief 일반 정보성 메시지를 화면에 표시합니다.
     * @param message 표시할 메시지.
     */
    void displayMessage(const std::string& message);

    /**
     * @brief 오류 메시지를 화면에 표시합니다. (주로 ErrorService의 결과를 받아 사용)
     * @param errorMessage 표시할 오류 메시지.
     */
    void displayError(const std::string& errorMessage);

    // --- 음료 제공 관련 UI ---
    /**
     * @brief 음료가 배출 중임을 알리는 메시지를 표시합니다. (UC7, UC14)
     * @param drinkName 배출 중인 음료의 이름.
     */
    void displayDispensingDrink(const std::string& drinkName);

    /**
     * @brief 음료 배출이 완료되었음을 알립니다. (UC7, UC14)
     * @param drinkName 배출된 음료의 이름.
     */
    void displayDrinkDispensed(const std::string& drinkName);

private:
    // Helper function for robust integer input from std::cin
    int getIntegerInput(const std::string& prompt, int minVal, int maxVal);
};

} // namespace presentation