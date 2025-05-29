#include "presentation/UserInterface.hpp" // .hpp로 가정

#include <iostream>  // std::cout, std::cin
#include <string>
#include <vector>
#include <limits>    // std::numeric_limits
#include <algorithm> // std::transform, std::all_of, ::toupper, ::isdigit
#include <iomanip>   // std::setw, std::left

namespace presentation {

/**
 * @brief 사용자로부터 지정된 범위 내의 정수 입력을 안전하게 받습니다.
 * 잘못된 입력 시 재입력을 요청합니다.
 * @param prompt 사용자에게 보여줄 안내 메시지.
 * @param minVal 입력 가능한 최소 정수값.
 * @param maxVal 입력 가능한 최대 정수값.
 * @return 사용자가 입력한 유효한 정수.
 */
int UserInterface::getIntegerInput(const std::string& prompt, int minVal, int maxVal) {
    int choice = 0;
    while (true) {
        std::cout << prompt;
        std::cin >> choice;
        if (std::cin.good() && choice >= minVal && choice <= maxVal) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 다음 입력을 위해 버퍼 비우기
            return choice;
        } else {
            std::cout << "잘못된 입력입니다. " << minVal << "부터 " << maxVal << " 사이의 숫자를 입력해주세요." << std::endl;
            std::cin.clear(); // 오류 플래그 초기화
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 잘못된 입력 내용 비우기
        }
    }
}

// --- 메뉴 및 선택 관련 ---

void UserInterface::displayMainMenu(const std::vector<std::string>& options) {
    std::cout << "\n옵션을 선택하세요:" << std::endl;
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << options[i] << std::endl; // 예: "1. 음료 선택 (UC2)"
    }
}

int UserInterface::getUserChoice(int maxChoice) {
    // UC1.3: 시스템은 사용자에게 음료를 선택할 수 있는 화면을 제공하고 입력을 기다리며 대기한다.
    return getIntegerInput("선택 (숫자 입력): ", 1, maxChoice);
}

std::string UserInterface::getUserInputString(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    // std::cin.ignore()를 호출한 후 getline을 사용하거나,
    // std::ws를 사용하여 앞쪽 공백을 제거할 수 있음.
    // 이전 getUserChoice에서 ignore를 했으므로 여기서는 바로 getline 가능.
    std::getline(std::cin, input);
    return input;
}

// --- 음료 관련 ---

void UserInterface::displayDrinkList(const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory) {
    // UC1.1 & UC1.2: 전체 음료 목록을 화면에 표시하고, 사용자가 확인.
    std::cout << "\n--- 음료 목록 (총 " << allDrinks.size() << "종) ---" << std::endl;
    std::cout << "+----------+--------------------+----------+" << std::endl;
    std::cout << "| " << std::left << std::setw(8) << "음료코드" << " | "
              << std::left << std::setw(18) << "음료명" << " | "
              << std::left << std::setw(8) << "가격" << " |" << std::endl;
    std::cout << "+----------+--------------------+----------+" << std::endl;

    if (allDrinks.empty()) {
        // UC1 E1에 대한 사용자 알림 (ErrorService에서도 오류 메시지 표시 가능)
        std::cout << "| 현재 판매 가능한 음료 정보를 불러올 수 없습니다.         |" << std::endl;
    } else {
        for (const auto& drink : allDrinks) {
            std::cout << "| " << std::left << std::setw(8) << drink.getDrinkCode() << " | "
                      << std::left << std::setw(18) << drink.getName() << " | "
                      << std::right << std::setw(7) << drink.getPrice() << "원 |" << std::endl;
        }
    }
    std::cout << "+----------+--------------------+----------+" << std::endl;
}

std::string UserInterface::selectDrink(const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory) {
    std::string drinkCodeInput;
    // UC2.1: 사용자가 화면에서 원하는 음료를 선택한다.
    while (true) {
        std::cout << "구매할 음료의 2자리 코드를 입력하세요 (메뉴로 돌아가려면 'c' 또는 'C' 입력): ";
        std::getline(std::cin, drinkCodeInput); // 공백 포함 가능, enter로 종료

        drinkCodeInput.erase(0, drinkCodeInput.find_first_not_of(" \t\n\r\f\v"));
        drinkCodeInput.erase(drinkCodeInput.find_last_not_of(" \t\n\r\f\v") + 1);

        if (drinkCodeInput == "c" || drinkCodeInput == "C") {
            return ""; // 사용자가 취소
        }

        bool isValidFormat = (drinkCodeInput.length() == 2 && std::all_of(drinkCodeInput.begin(), drinkCodeInput.end(), ::isdigit));
        bool existsInList = false;
        if (isValidFormat) {
            for (const auto& drink : allDrinks) {
                if (drink.getDrinkCode() == drinkCodeInput) {
                    existsInList = true;
                    break;
                }
            }
        }

        if (isValidFormat && existsInList) {
            return drinkCodeInput;
        } else {
            std::cout << "잘못된 음료 코드입니다. 목록에 있는 2자리 숫자 코드를 입력하거나 'c'를 입력하여 취소하세요." << std::endl;
        }
    }
}

// --- 결제 관련 ---
void UserInterface::displayPaymentPrompt(int amount) {
    // UC4.1: 사용자에게 화면으로 결제 요청을 안내한다.
    std::cout << "\n결제할 금액: " << amount << "원 입니다." << std::endl;
    std::cout << "카드를 삽입해주세요. (시뮬레이션: 실제 카드 처리 없음)" << std::endl;
}

bool UserInterface::confirmPayment() {
    // UC4.2 (사용자 결제 시도 의사 확인) / UC11.2 (선결제 동의 확인)
    std::string input;
    while (true) {
        std::cout << "결제를 계속 진행하시겠습니까? (Y/N): ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            char choice = std::toupper(input[0]);
            if (choice == 'Y') return true;
            if (choice == 'N') return false;
        }
        std::cout << "잘못된 입력입니다. Y 또는 N을 입력해주세요." << std::endl;
    }
}

void UserInterface::displayPaymentProcessing() {
    std::cout << "결제 승인 중입니다. 잠시만 기다려주세요..." << std::endl;
}

void UserInterface::displayPaymentResult(bool success, const std::string& message) {
    // UC5.3 또는 UC6.3 결과 메시지 표시
    if (success) {
        std::cout << "[결제 성공] " << message << std::endl;
    } else {
        std::cout << "[결제 실패] " << message << std::endl;
    }
}

// --- 선결제 관련 ---
bool UserInterface::confirmPrepayment(const std::string& drinkName) {
    // UC11.1: 시스템은 사용자에게 선결제 여부를 묻는다
    std::string input;
    while (true) {
        std::cout << "[" << drinkName << "] 음료에 대해 다른 자판기에서 선결제 하시겠습니까? (Y/N): ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            char choice = std::toupper(input[0]);
            if (choice == 'Y') return true; // UC11.2: 사용자가 선결제 수행 결정
            if (choice == 'N') return false;
        }
        std::cout << "잘못된 입력입니다. Y 또는 N을 입력해주세요." << std::endl;
    }
}

void UserInterface::displayAuthCode(const std::string& authCode, const domain::VendingMachine& targetVm, const std::string& drinkName) {
    // UC12.2 & UC12.3: AuthCode를 사용자에게 표시하고 안내 메시지를 표시한다.
    // PFR: 선결제를 마친 후 사용자에게 인증코드와 선결제를 마친 자판기의 위치 정보(좌표) 등 필요한 정보를 안내해야 한다.
    std::cout << "\n==== 선결제 완료 ====" << std::endl;
    std::cout << "  음료명: " << drinkName << std::endl;
    std::cout << "  수령 자판기 ID: " << targetVm.getId() << std::endl;
    std::cout << "  수령 자판기 위치: (X: " << targetVm.getLocation().first
              << ", Y: " << targetVm.getLocation().second << ")" << std::endl;
    std::cout << "  ★ 인증 코드: " << authCode << " ★" << std::endl;
    std::cout << "====================" << std::endl;
    std::cout << "위 자판기에서 발급된 인증 코드를 입력하여 음료를 수령하세요." << std::endl;
}

std::string UserInterface::getAuthCodeInput() {
    std::string authCode;
    // UC13.1 & UC13.2: 사용자가 인증코드 입력 화면으로 이동하고 인증코드를 입력한다.
    while (true) {
        std::cout << "수령할 음료의 5자리 인증 코드를 입력하세요 (메인 메뉴: c 또는 C): ";
        std::getline(std::cin, authCode);

        authCode.erase(0, authCode.find_first_not_of(" \t\n\r\f\v"));
        authCode.erase(authCode.find_last_not_of(" \t\n\r\f\v") + 1);

        if (authCode == "c" || authCode == "C") {
            return ""; // 사용자가 취소
        }
        // PFR: 인증코드는 알파벳 대/소문자와 숫자를 포함하는 랜덤한 5자리의 문자열
        // UC13.3: 입력 문자열이 정규식 “[A-Za-z0-9]{5}”와 일치하는지 검사한다. (형식 검사는 PrepaymentService에서도 수행)
        if (authCode.length() == 5 && std::all_of(authCode.begin(), authCode.end(), ::isalnum)) {
            return authCode;
        } else {
            std::cout << "잘못된 형식의 인증 코드입니다. 5자리 영숫자를 입력해주세요." << std::endl;
        }
    }
}

// --- 자판기 및 재고 관련 상태 메시지 ---
void UserInterface::displayOutOfStockMessage(const std::string& drinkName) {
    // UC3.3에서 호출
    std::cout << "알림: 선택하신 음료 [" << drinkName << "]는 현재 자판기에 재고가 없습니다." << std::endl;
}

void UserInterface::displayNearestVendingMachine(const domain::VendingMachine& vm, const std::string& drinkName) {
    // UC10.1: 가장 가까운 자판기를 계산해 위치정보를 사용자에게 안내한다.
    std::cout << "\n음료 [" << drinkName << "]를 구매 가능한 가장 가까운 자판기 정보입니다:" << std::endl;
    std::cout << "  > 자판기 ID: " << vm.getId() << std::endl;
    std::cout << "  > 위    치: (X: " << vm.getLocation().first << ", Y: " << vm.getLocation().second << ")" << std::endl;
}

void UserInterface::displayNoOtherVendingMachineFound(const std::string& drinkName) {
    // UC9 E2 (타임아웃 시 응답 없음) 또는 UC10 (가까운 자판기 없음)
    std::cout << "안내: 주변 다른 자판기에서 음료 [" << drinkName << "]의 재고를 찾을 수 없었습니다." << std::endl;
}

// --- 일반 메시지 및 오류 ---
void UserInterface::displayMessage(const std::string& message) {
    std::cout << message << std::endl;
}

void UserInterface::displayError(const std::string& errorMessage) {
    // UserProcessController::state_handlingError에서 호출
    std::cerr << "[시스템 오류] " << errorMessage << std::endl;
}

// --- 음료 제공 관련 UI ---
void UserInterface::displayDispensingDrink(const std::string& drinkName) {
    // UC7.1 또는 UC14 이후
    std::cout << "\n[" << drinkName << "] 음료가 배출되고 있습니다. 잠시만 기다려주세요..." << std::endl;
}

void UserInterface::displayDrinkDispensed(const std::string& drinkName) {
    // UC7.3
    std::cout << "[" << drinkName << "] 음료가 나왔습니다. 이용해주셔서 감사합니다!" << std::endl;
}

} // namespace presentation