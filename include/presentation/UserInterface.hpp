#ifndef USER_INTERFACE_HPP
#define USER_INTERFACE_HPP

#include <vector>
#include <string>

class UserInterface {
public:
    void displayMainMenu();
    void displayList(const std::vector<std::pair<std::string, int>>& list);
    void show_error_message(const std::string& error);
    void display_Error(const std::string& error);
    void showText(const std::string& prepaymentCode);
    static void displayMessage(const std::string& msg);
    void promptCardInfo();
    std::string promptPrepayCode();
    bool promptPrepayConsent();
    void display_SomeText(const std::string& text);
    bool isValidCardNumber(const std::string& cardNumber);


    // ?�� UC7: ?���?? 배출 메시�??
    void dispense(const std::string& drinkCode);
};

#endif
