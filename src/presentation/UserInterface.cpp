#include "presentation/UserInterface.hpp"
#include <iostream>
#include "service/UserProcessController.hpp"


void UserInterface::displayMainMenu() {
    std::cout << "===== 占쎌쁽占쎈솇疫뀐옙 筌롫뗀�뤀 =====\n";
    std::cout << "1. 占쎌벉�뙴占� 筌뤴뫖以� 鈺곌퀬�돳\n2. 占쎌벉�뙴占� 占쎄퐨占쎄문\n3. 占쎌삺��⑨옙 占쎌넇占쎌뵥\n4. �넫�굝利�\n5. 占쎌뵥筌앹빘�맜占쎈굡 占쎌뿯占쎌젾\n甕곕뜇�깈�몴占� 占쎄퐨占쎄문占쎈릭占쎄쉭占쎌뒄: ";
}

void UserInterface::displayList(const std::vector<std::pair<std::string, int>>& list) {
    std::cout << "[占쎌벉�뙴占� 筌뤴뫖以�]\n";
    for (const auto& drink : list) {
        std::cout << "占쎌뵠�뵳占�: " << drink.first << ", 揶쏉옙野껓옙: " << drink.second << "占쎌뜚\n";
    }
}

void UserInterface::show_error_message(const std::string& error) {
    std::cerr << "[占쎈퓠占쎌쑎 筌롫뗄�뻻筌욑옙] " << error << std::endl;
}

void UserInterface::display_Error(const std::string& error) {
    std::cerr << "[占쎈ご占쎈뻻 占쎌궎�몴占�] " << error << std::endl;
}

void UserInterface::displayMessage(const std::string& msg) {
    std::cout << msg << std::endl;
}

void UserInterface::promptCardInfo() {


    UserProcessController controller;
    // handlePayment 占쎌깈�빊占�
    controller.handlePayment(false); //UI�떒�뿉�꽌 �꽑寃곗젣��� �씪諛섍껐�젣瑜� 援щ텇�븯吏� 紐삵븿 -> �씠嫄� 援ъ“ 
}

std::string UserInterface::promptPrepayCode() {
    std::string code;
    std::cout << "占쎌뵥筌앹빘�맜占쎈굡�몴占� 占쎌뿯占쎌젾占쎈릭占쎄쉭占쎌뒄: ";
    std::cin >> code;
    return code;
}

void UserInterface::dispense(const std::string& drinkCode) {
    std::cout << "[占쎌벉�뙴占� 獄쏄퀣�뀱] �굜遺얜굡 " << drinkCode << " 占쎈퓠 占쎈퉸占쎈뼣占쎈릭占쎈뮉 占쎌벉�뙴�슡占쏙옙 獄쏄퀣�뀱占쎈┷占쎈��占쎈뮸占쎈빍占쎈뼄.\n";
}


bool UserInterface::promptPrepayConsent() {
   //"[占쎄퐨野껉퀣�젫 占쎈짗占쎌벥] 占쎄퐨野껉퀣�젫 占쎈짗占쎌벥 占쎈연�겫占썹몴占� 占쎌뿯占쎌젾占쎈릭占쎄쉭占쎌뒄 (y/n) yes -> true 獄쏆꼹�넎 
    char consent;
    std::cout << "占쎄퐨野껉퀣�젫 占쎈짗占쎌벥 占쎈연�겫占썹몴占� 占쎌뿯占쎌젾占쎈릭占쎄쉭占쎌뒄 (y/n): ";
    std::cin >> consent;
    return (consent == 'y' || consent == 'Y');
    }

void UserInterface::showText(const std::string& prepaymentCode) {
    std::cout << "占쎄퐨野껉퀣�젫 �굜遺얜굡: " << prepaymentCode << std::endl;
}

void UserInterface::display_SomeText(const std::string& text) {
    std::cout << text << std::endl;
}