#include "presentation/UserInterface.hpp" // UserInterface.h -> UserInterface.hpp 로 가정
#include "domain/drink.h"             // domain::Drink 사용 (hpp로 가정)
#include "domain/inventory.h"         // domain::Inventory 사용 (hpp로 가정)
#include "domain/vendingMachine.h"    // domain::VendingMachine 사용 (hpp로 가정)

#include <iostream>
#include <string>
#include <vector>
#include <limits>    // std::numeric_limits
#include <algorithm> // std::transform, std::all_of
#include <iomanip>   // std::setw, std::left

// Helper function for robust integer input (UserInterface.hpp private 선언 부분)
// UserInterface.cpp 내부에 static으로 두거나, private 멤버 함수로 구현.
// 여기서는 private 멤버 함수로 가정하고 UserInterface::getIntegerInput 으로 호출.
int presentation::UserInterface::getIntegerInput(const std::string& prompt, int minVal, int maxVal) {
    int choice = 0;
    while (true) {
        std::cout << prompt;
        std::cin >> choice;
        if (std::cin.good() && choice >= minVal && choice <= maxVal) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 입력 버퍼 비우기
            return choice;
        } else {
            std::cout << "잘못된 입력입니다. " << minVal << "부터 " << maxVal << " 사이의 숫자를 입력해주세요." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

namespace presentation {

// --- 메뉴 및 선택 관련 ---
void UserInterface::displayMainMenu(const std::vector<std::string>& options) {
    std::cout << "\n옵션을 선택하세요:" << std::endl;
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << options[i] << std::endl;
    }
}

int UserInterface::getUserChoice(int maxChoice) {
    return getIntegerInput("선택: ", 1, maxChoice);
}

std::string UserInterface::getUserInputString(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input); // std::ws 없이 사용하면 이전 개행문자 영향 받을 수 있음.
                                   // 필요시 std::cin >> std::ws >> input; 또는
                                   // 이전 입력 후 std::cin.ignore() 확실히 하기.
                                   // 여기서는 getUserChoice 이후 호출된다고 가정하고 단순 getline.
    return input;
}

// --- 음료 관련 ---
void UserInterface::displayDrinkList(const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory) {
    std::cout << "\n--- 음료 목록 ---" << std::endl;
    std::cout << std::left << std::setw(10) << "음료 코드"
              << std::left << std::setw(20) << "음료명"
              << std::left << std::setw(10) << "가격" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    if (allDrinks.empty()) {
        std::cout << "판매 중인 음료가 없습니다." << std::endl;
    } else {
        for (const auto& drink : allDrinks) {
            std::cout << std::left << std::setw(10) << drink.getDrinkCode()
                      << std::left << std::setw(20) << drink.getName()
                      << std::left << std::setw(10) << drink.getPrice() << "원" << std::endl;
        }
    }
    std::cout << "----------------------------------------" << std::endl;
    // currentVmInventory를 사용하여 현재 자판기 재고 상태를 추가로 표시할 수 있으나,
    // 현재 UserProcessController에서는 모든 음료 목록만 표시하도록 간소화되어 있음.
}

std::string UserInterface::selectDrink(const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory) {
    std::string drinkCode;
    // displayDrinkList(allDrinks, currentVmInventory); // 이미 메인 메뉴에서 표시했을 수 있음. 필요시 다시 표시.
    while (true) {
        std::cout << "구매할 음료의 코드를 입력하세요 (취소: c): ";
        std::cin >> drinkCode;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 입력 버퍼 비우기

        if (drinkCode == "c" || drinkCode == "C") {
            return ""; // 취소 시 빈 문자열 반환
        }

        // 입력된 코드가 유효한 형식인지 (예: 숫자 2자리) 및 목록에 있는지 확인
        bool isValidCode = false;
        if (drinkCode.length() == 2 && std::all_of(drinkCode.begin(), drinkCode.end(), ::isdigit)) {
            for (const auto& drink : allDrinks) {
                if (drink.getDrinkCode() == drinkCode) {
                    isValidCode = true;
                    break;
                }
            }
        }

        if (isValidCode) {
            return drinkCode;
        } else {
            std::cout << "잘못된 음료 코드입니다. 목록에 있는 2자리 숫자 코드를 입력해주세요." << std::endl;
        }
    }
}

// --- 결제 관련 ---
void UserInterface::displayPaymentPrompt(int amount) {
    std::cout << "\n결제할 금액: " << amount << "원" << std::endl;
    std::cout << "카드를 삽입하고 결제를 진행해주세요." << std::endl;
    // 실제 카드 리더기 상호작용은 없으므로 메시지만 표시
}

bool UserInterface::confirmPayment() {
    std::string input;
    while (true) {
        std::cout << "결제를 진행하시겠습니까? (Y/N): ";
        std::cin >> input;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::transform(input.begin(), input.end(), input.begin(), ::toupper);
        if (input == "Y") return true;
        if (input == "N") return false;
        std::cout << "잘못된 입력입니다. Y 또는 N을 입력해주세요." << std::endl;
    }
}

void UserInterface::displayPaymentProcessing() {
    std::cout << "결제 진행 중입니다. 잠시만 기다려주세요..." << std::endl;
}

void UserInterface::displayPaymentResult(bool success, const std::string& message) {
    if (success) {
        std::cout << "[결제 성공] " << message << std::endl;
    } else {
        std::cout << "[결제 실패] " << message << std::endl;
    }
}

// --- 선결제 관련 ---
bool UserInterface::confirmPrepayment(const std::string& drinkName) {
    std::string input;
    while (true) {
        std::cout << drinkName << " 음료를 다른 자판기에서 수령하기 위해 선결제 하시겠습니까? (Y/N): ";
        std::cin >> input;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::transform(input.begin(), input.end(), input.begin(), ::toupper);
        if (input == "Y") return true;
        if (input == "N") return false;
        std::cout << "잘못된 입력입니다. Y 또는 N을 입력해주세요." << std::endl;
    }
}

void UserInterface::displayAuthCode(const std::string& authCode, const domain::VendingMachine& targetVm, const std::string& drinkName) {
    std::cout << "\n--- 선결제 완료 ---" << std::endl;
    std::cout << "음료: " << drinkName << std::endl;
    std::cout << "수령 자판기 ID: " << targetVm.getId() << std::endl;
    std::cout << "수령 자판기 위치: X=" << targetVm.getLocation().first << ", Y=" << targetVm.getLocation().second << std::endl;
    std::cout << "인증 코드: " << authCode << std::endl;
    std::cout << "위 자판기에서 인증 코드를 입력하고 음료를 수령하세요." << std::endl;
}

std::string UserInterface::getAuthCodeInput() {
    std::string authCode;
    while (true) {
        std::cout << "5자리 인증 코드를 입력하세요 (취소: c): ";
        std::cin >> authCode;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (authCode == "c" || authCode == "C") {
            return ""; // 취소
        }
        if (authCode.length() == 5 && std::all_of(authCode.begin(), authCode.end(), ::isalnum)) {
            return authCode;
        } else {
            std::cout << "잘못된 형식입니다. 5자리 영숫자 코드를 입력해주세요." << std::endl;
        }
    }
}

// --- 자판기 및 재고 관련 ---
void UserInterface::displayOutOfStockMessage(const std::string& drinkName) {
    std::cout << "선택하신 음료 [" << drinkName << "]는 현재 자판기에 재고가 없습니다." << std::endl;
}

void UserInterface::displayNearestVendingMachine(const domain::VendingMachine& vm, const std::string& drinkName) {
    std::cout << "\n선택하신 음료 [" << drinkName << "]는 현재 자판기에 없습니다." << std::endl;
    std::cout << "가장 가까운 구매 가능 자판기 정보:" << std::endl;
    std::cout << "  자판기 ID: " << vm.getId() << std::endl;
    std::cout << "  위치: X=" << vm.getLocation().first << ", Y=" << vm.getLocation().second << std::endl;
    // 포트 정보는 사용자에게 직접 보여줄 필요는 없을 수 있음
}

void UserInterface::displayNoOtherVendingMachineFound(const std::string& drinkName) {
    std::cout << "주변 다른 자판기에서 음료 [" << drinkName << "]의 재고를 찾을 수 없었습니다." << std::endl;
}

// --- 일반 메시지 및 오류 ---
void UserInterface::displayMessage(const std::string& message) {
    std::cout << message << std::endl;
}

void UserInterface::displayError(const std::string& errorMessage) {
    std::cerr << "[오류] " << errorMessage << std::endl;
}

// --- 음료 제공 관련 ---
void UserInterface::displayDispensingDrink(const std::string& drinkName) {
    std::cout << "\n" << drinkName << " 음료를 배출 중입니다. 잠시만 기다려주세요..." << std::endl;
}

void UserInterface::displayDrinkDispensed(const std::string& drinkName) {
    std::cout << drinkName << " 음료가 나왔습니다. 맛있게 드세요!" << std::endl;
}

} // namespace presentation