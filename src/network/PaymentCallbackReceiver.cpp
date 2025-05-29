#include "network/PaymentCallbackReceiver.hpp"

void network::PaymentCallbackReceiver::simulatePrepayment(Callback cb,int delay_seconds) const {
    std::this_thread::sleep_for(std::chrono::seconds(delay_seconds));
    static thread_local std::mt19937 gen{std::random_device{}()};
    std::uniform_real_distribution<> dist(0.0, 1.0);
    bool success = dist(gen) < 0.99;
    cb(success);
}