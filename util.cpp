#include <cmath>
#include <chrono>
#include <thread>
#include "util.h"

int round_int(const double n) {
    return static_cast<int>(round(n));
}

void sleep(const double duration_s) {
    const unsigned int duration_ns = static_cast<unsigned int>(round(duration_s * 1000000000));
    std::this_thread::sleep_for(std::chrono::nanoseconds(duration_ns));
}
