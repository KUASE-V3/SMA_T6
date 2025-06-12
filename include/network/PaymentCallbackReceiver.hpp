#pragma once

#include <functional> 
#include <thread>     
#include <chrono>     
#include <random>    
namespace network {
class PaymentCallbackReceiver {
public:
    using Callback = std::function<void(bool success)>;

    template <typename T_Callback> // 템플릿 파라미터 사용
    void simulatePrepayment(const T_Callback& cb, int delay_seconds = 3) const {
        std::this_thread::sleep_for(std::chrono::seconds(delay_seconds));
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dist(0.0, 1.0);
        bool success = dist(gen) < 0.99;
        cb(success);
    }
};

} // namespace network
