#pragma once

#include <chrono>

namespace RDE{
    class Ticker {
    public:
        Ticker() : m_start_time(std::chrono::high_resolution_clock::now()) {}

        float tick() {
            auto now = std::chrono::high_resolution_clock::now();
            float elapsed = std::chrono::duration<float>(now - m_start_time).count();
            m_start_time = now; // Reset start time for next tick
            return elapsed;
        }
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
    };
}