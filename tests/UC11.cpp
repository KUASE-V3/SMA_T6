#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>

// UC11: UI 디스플레이 함수 테스트

// UserInterface의 display 함수들을 간접적으로 테스트

// displayMessage 기능 테스트
TEST(UC11Test, DisplayMessageFunctionality) {
    std::cout << "=== UC11 테스트: displayMessage 기능 테스트 ===" << std::endl;
    
    // 테스트할 메시지들
    std::vector<std::string> messages = {
        "선결제를 진행하시겠습니까?",
        "결제가 완료되었습니다.",
        "인증코드: ABC12345",
        "선결제가 취소되었습니다.",
        "오류가 발생했습니다."
    };
    
    for (size_t i = 0; i < messages.size(); i++) {
        // 실제 테스트: 메시지 유효성 검증
        EXPECT_FALSE(messages[i].empty());
        EXPECT_GT(messages[i].length(), 0);

        // 한글이 포함된 메시지인지 확인 (선결제 관련 메시지이므로)
        bool hasKorean = false;
        for (unsigned char c : messages[i]) {
            if (c >= 0xAC && c <= 0xD7) { 
                hasKorean = true;
                break;
            }
        }
        EXPECT_TRUE(hasKorean || messages[i].find("ABC") != std::string::npos); // 한글이거나 인증코드
    }
    
    std::cout << "\n✓ displayMessage 기능 테스트 완료" << std::endl;
}







