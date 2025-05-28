#pragma once

#include <string>
#include <vector>
#include "domain/drink.h"
#include "domain/inventory.h"
#include "domain/vendingMachine.h"

namespace presentation {

/**
 * @brief ����� �������̽�(CLI ���)�� ����ϴ� Ŭ�����Դϴ�.
 * ����ڷκ��� �Է��� �ް�, �ý����� ������ ȭ�鿡 ǥ���մϴ�.
 */
class UserInterface {
public:
    UserInterface() = default;

    // --- �޴� �� �Ϲ� �Է� ���� ---
    /**
     * @brief ���� �޴� �ɼǵ��� ȭ�鿡 ǥ���մϴ�.
     * @param options ǥ���� �޴� �ɼ� ���ڿ� ����.
     */
    void displayMainMenu(const std::vector<std::string>& options);

    /**
     * @brief ����ڷκ��� �޴� ����(����)�� �Է¹ް� ��ȿ���� �˻��մϴ�.
     * @param maxChoice ���� ������ �ִ� �޴� ��ȣ.
     * @return ����ڰ� ������ ��ȿ�� �޴� ��ȣ.
     */
    int getUserChoice(int maxChoice);

    /**
     * @brief ����ڷκ��� �Ϲ� ���ڿ� �Է��� �޽��ϴ�.
     * @param prompt ����ڿ��� ������ �ȳ� �޽���.
     * @return ����ڰ� �Է��� ���ڿ�.
     */
    std::string getUserInputString(const std::string& prompt);

    // --- ���� ���� ǥ�� �� ���� ---
    /**
     * @brief ��� ���� ����� ȭ�鿡 ǥ���մϴ�. (UC1)
     * @param allDrinks ǥ���� ��ü ���� ��� (domain::Drink ��ü�� ����).
     * @param currentVmInventory ���� ���Ǳ��� ��� ���� (�̹� ���������� ���� ��� ǥ�ÿ� ���� ������ ���� �� ����).
     */
    void displayDrinkList(const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory);

    /**
     * @brief ����ڷκ��� ������ ������ �ڵ带 �Է¹޽��ϴ�. (UC2)
     * @param allDrinks ���� ������ ��ü ���� ��� (�ڵ� ��ȿ�� �˻��).
     * @param currentVmInventory ���� ���Ǳ� ��� (�����, ���� ��� �� ��).
     * @return ����ڰ� �Է��� ���� �ڵ� (��ȿ�ϰų�, ��� �� �� ���ڿ�).
     */
    std::string selectDrink(const std::vector<domain::Drink>& allDrinks, const std::vector<domain::Inventory>& currentVmInventory);

    // --- ���� ���� UI ---
    /**
     * @brief ������ �ݾװ� �Բ� ī�� ������ ��û�ϴ� �޽����� ǥ���մϴ�. (UC4)
     * @param amount ������ �ݾ�.
     */
    void displayPaymentPrompt(int amount);

    /**
     * @brief ����ڿ��� ������ ��� �������� ���θ� Ȯ�ι޽��ϴ�. (Y/N)
     * @return ����ڰ� �����ϸ� true, �׷��� ������ false.
     */
    bool confirmPayment();

    /**
     * @brief "���� ���� ��" �޽����� ǥ���մϴ�. (UC4)
     */
    void displayPaymentProcessing();

    /**
     * @brief ���� ����� ����ڿ��� ǥ���մϴ�. (UC5, UC6)
     * @param success ���� ���� ����.
     * @param message ����� �Բ� ǥ���� �߰� �޽���.
     */
    void displayPaymentResult(bool success, const std::string& message);

    // --- ������ ���� UI ---
    /**
     * @brief Ư�� ���ῡ ���� �ٸ� ���Ǳ⿡�� ���������� ���θ� ����ڿ��� Ȯ�ι޽��ϴ�. (UC11)
     * @param drinkName ������ ��� ������ �̸�.
     * @return ����ڰ� �����ϸ� true, �׷��� ������ false.
     */
    bool confirmPrepayment(const std::string& drinkName);

    /**
     * @brief �߱޵� ���� �ڵ�� ���� ���Ǳ� ������ ����ڿ��� �ȳ��մϴ�. (UC12)
     * @param authCode �߱޵� ���� �ڵ�.
     * @param targetVm ���Ḧ ������ ��� ���Ǳ� ����.
     * @param drinkName �������� ������ �̸�.
     */
    void displayAuthCode(const std::string& authCode, const domain::VendingMachine& targetVm, const std::string& drinkName);

    /**
     * @brief ����ڷκ��� ���� �ڵ带 �Է¹޽��ϴ�. (UC13)
     * @return ����ڰ� �Է��� ���� �ڵ� (���� �˻�� ���� �������� ����). ��� �� �� ���ڿ�.
     */
    std::string getAuthCodeInput();

    // --- ���Ǳ� �� ��� ���� ���� �޽��� ---
    /**
     * @brief ������ ���ᰡ ���� ���Ǳ⿡ ��� ������ �˸��ϴ�. (UC3)
     * @param drinkName ��� ���� ������ �̸�.
     */
    void displayOutOfStockMessage(const std::string& drinkName);

    /**
     * @brief ���� ������ ���� ����� �ٸ� ���Ǳ��� ������ �ȳ��մϴ�. (UC10)
     * @param vm ���� ����� ���Ǳ� ����.
     * @param drinkName ã�� �ִ� ������ �̸�.
     */
    void displayNearestVendingMachine(const domain::VendingMachine& vm, const std::string& drinkName);

    /**
     * @brief �ֺ� �ٸ� ���Ǳ⿡�� �ش� ������ ��� ã�� �� ������ �˸��ϴ�. (UC9 Ÿ�Ӿƿ� ��)
     * @param drinkName ã�� �ִ� ������ �̸�.
     */
    void displayNoOtherVendingMachineFound(const std::string& drinkName);

    // --- �Ϲ� �޽��� �� ���� �޽��� ---
    /**
     * @brief �Ϲ� ������ �޽����� ȭ�鿡 ǥ���մϴ�.
     * @param message ǥ���� �޽���.
     */
    void displayMessage(const std::string& message);

    /**
     * @brief ���� �޽����� ȭ�鿡 ǥ���մϴ�. (�ַ� ErrorService�� ����� �޾� ���)
     * @param errorMessage ǥ���� ���� �޽���.
     */
    void displayError(const std::string& errorMessage);

    // --- ���� ���� ���� UI ---
    /**
     * @brief ���ᰡ ���� ������ �˸��� �޽����� ǥ���մϴ�. (UC7, UC14)
     * @param drinkName ���� ���� ������ �̸�.
     */
    void displayDispensingDrink(const std::string& drinkName);

    /**
     * @brief ���� ������ �Ϸ�Ǿ����� �˸��ϴ�. (UC7, UC14)
     * @param drinkName ����� ������ �̸�.
     */
    void displayDrinkDispensed(const std::string& drinkName);

private:
    // Helper function for robust integer input from std::cin
    int getIntegerInput(const std::string& prompt, int minVal, int maxVal);
};

} // namespace presentation