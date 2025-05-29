// src/network/PaymentCallbackReceiver.cpp
#include "network/PaymentCallbackReceiver.hpp"
#include <thread>        // std::this_thread::sleep_for
#include <chrono>        // std::chrono::seconds
#include <random>        // std::random_device, std::mt19937, std::uniform_real_distribution

void network::PaymentCallbackReceiver::simulatePrepayment(Callback cb, int delay_seconds) const {
    std::this_thread::sleep_for(std::chrono::seconds(delay_seconds));

    // 매 호출 시 std::random_device로 시드된 mt19937 사용
    // 이는 static thread_local로 하나의 인스턴스를 계속 사용하는 것보다
    // 예측 가능성을 아주 약간 낮출 수 있지만, 여전히 CSPRNG는 아닙니다.
    std::random_device rd; // 가능한 경우 하드웨어 엔트로피 소스 사용 시도
    std::mt19937 gen(rd()); // 매번 새로운 시드로 mt19937 인스턴스화
    
    std::uniform_real_distribution<> dist(0.0, 1.0);
    bool success = dist(gen) < 0.99; // 99% 확률로 성공
    
    cb(success);
}