#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include "domain/drink.h"
#include "domain/inventory.h"
#include "domain/vendingMachine.h"

namespace presentation {

class UserInterface {
public:
    UserInterface() = default;

    void displayMainMenu(const std::vector<std::string>& options);
    int getUserChoice(int maxChoice);
    std::string getUserInputString(const std::string& prompt);
    void displayDrinkList(const std::vector<domain::Drink>& allDrinks);
    std::string selectDrink(const std::vector<domain::Drink>& allDrinks);
    void displayPaymentPrompt(int amount);
    bool confirmPayment(std::chrono::seconds timeout);
    void displayPaymentProcessing();
    void displayPaymentResult(bool success, const std::string& message);
    bool confirmPrepayment(const std::string& drinkName, std::chrono::seconds timeout);
    void displayAuthCode(const std::string& authCode, const domain::VendingMachine& targetVm, const std::string& drinkName);
    std::string getAuthCodeInput();
    void displayOutOfStockMessage(const std::string& drinkName);
    void displayNearestVendingMachine(const domain::VendingMachine& vm, const std::string& drinkName);
    void displayNoOtherVendingMachineFound(const std::string& drinkName);
    void displayMessage(const std::string& message);


    void displayError(const std::string& errorMessage);

    void displayDispensingDrink(const std::string& drinkName);

    void displayDrinkDispensed(const std::string& drinkName);

private:
    int getIntegerInput(const std::string& prompt, int minVal, int maxVal);
    bool isDrinkCodeValid(const std::string& code, const std::vector<domain::Drink>& allDrinks) const;

};

} // namespace presentation