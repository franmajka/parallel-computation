#pragma once
#include <chrono>

class Timer {
public:
    using clock_t = std::chrono::high_resolution_clock;
    using second_t = std::chrono::duration<double, std::ratio<1>>;
    using millisecond_t = std::chrono::duration<double, std::milli>;
    using microsecond_t = std::chrono::duration<double, std::micro>;

    Timer() : m_beg(clock_t::now()) {}

    void reset() { m_beg = clock_t::now(); }

    template <typename DurT = second_t>
    double elapsed() const {
        return std::chrono::duration_cast<DurT>(clock_t::now() - m_beg).count();
    }

private:
    std::chrono::time_point<clock_t> m_beg;
};
