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

    // �޴� �� ���� ����
    void displayMainMenu(const std::vector<std::string>& options);
    int getUserChoice(int maxChoice);
    std::string getUserInputString(const std::string& prompt);

    // ���� ����
    void displayDrinkList(const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory);
    std::string selectDrink(const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory);

    // ���� ����
    void displayPaymentPrompt(int amount);
    bool confirmPayment(); // ����ڰ� ī�� ���� �õ� Ȯ�� (Y/N ��ȯ ����)
    void displayPaymentProcessing();
    void displayPaymentResult(bool success, const std::string& message);

    // ������ ����
    bool confirmPrepayment(const std::string& drinkName); // ������ ���� Ȯ�� (Y/N ��ȯ ����)
    void displayAuthCode(const std::string& authCode, const domain::VendingMachine& targetVm, const std::string& drinkName);
    std::string getAuthCodeInput();

    // ���Ǳ� �� ��� ����
    void displayOutOfStockMessage(const std::string& drinkName);
    void displayNearestVendingMachine(const domain::VendingMachine& vm, const std::string& drinkName);
    void displayNoOtherVendingMachineFound(const std::string& drinkName);

    // �Ϲ� �޽��� �� ����
    void displayMessage(const std::string& message);
    void displayError(const std::string& errorMessage);

    // ���� ���� ����
    void displayDispensingDrink(const std::string& drinkName);
    void displayDrinkDispensed(const std::string& drinkName);

private:
    // Helper function for robust integer input
    int getIntegerInput(const std::string& prompt, int minVal, int maxVal);
    // Helper function for drink code input validation (�ʿ�� �߰�)
    // std::string getValidatedDrinkCodeInput(const std::string& prompt, const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory, bool mustBeAvailable);
};

} // namespace presentation