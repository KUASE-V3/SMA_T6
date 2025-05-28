#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>


#include "domain/drink.h"
#include "domain/inventory.h"

namespace presentation {

class UserInterface {
public:
    UserInterface() = default;

    // 메뉴 및 선택 관련
    void displayMainMenu(const std::vector<std::string>& options);
    int getUserChoice(int maxChoice);
    std::string getUserInputString(const std::string& prompt);

    // 음료 관련
    void displayDrinkList(const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory);
    std::string selectDrink(const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory);

    // 결제 관련
    void displayPaymentPrompt(int amount);
    bool confirmPayment(); // 사용자가 카드 결제 시도 확인 (Y/N 반환 가정)
    void displayPaymentProcessing();
    void displayPaymentResult(bool success, const std::string& message);

    // 선결제 관련
    bool confirmPrepayment(const std::string& drinkName); // 선결제 여부 확인 (Y/N 반환 가정)
    void displayAuthCode(const std::string& authCode, const domain::VendingMachine& targetVm, const std::string& drinkName);
    std::string getAuthCodeInput();

    // 자판기 및 재고 관련
    void displayOutOfStockMessage(const std::string& drinkName);
    void displayNearestVendingMachine(const domain::VendingMachine& vm, const std::string& drinkName);
    void displayNoOtherVendingMachineFound(const std::string& drinkName);

    // 일반 메시지 및 오류
    void displayMessage(const std::string& message);
    void displayError(const std::string& errorMessage);

    // 음료 제공 관련
    void displayDispensingDrink(const std::string& drinkName);
    void displayDrinkDispensed(const std::string& drinkName);

private:
    // Helper function for robust integer input
    int getIntegerInput(const std::string& prompt, int minVal, int maxVal);
    // Helper function for drink code input validation (필요시 추가)
    // std::string getValidatedDrinkCodeInput(const std::string& prompt, const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory, bool mustBeAvailable);
};

} // namespace presentation