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
    void displayMessage(const std::string& msg);
    std::string promptCardInfo();
    std::string promptPrepayCode();
    bool promptPrepayConsent();


    // ?è∫ UC7: ?ùåÎ£? Î∞∞Ï∂ú Î©îÏãúÏß?
    void dispense(const std::string& drinkCode);
};

#endif
