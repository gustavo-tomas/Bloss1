#include "core/timer.hpp"

namespace bls
{
    Timer::Timer()
    {
        time = 0;
    }

    Timer::~Timer()
    {

    }

    void Timer::update(f32 dt)
    {
        time += dt;
    }

    void Timer::restart()
    {
        time = 0;
    }

    float Timer::get()
    {
        return time;
    }
};
