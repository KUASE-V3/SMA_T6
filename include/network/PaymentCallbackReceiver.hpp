#pragma once

#include <functional>
#include <thread>
#include <chrono>
#include <random>

namespace network {
class PaymentCallbackReceiver {
public:
    using Callback = std::function<void(bool success)>;
    void simulatePrepayment(Callback cb, int delay_seconds = 3) const;
};
} // namespace network
