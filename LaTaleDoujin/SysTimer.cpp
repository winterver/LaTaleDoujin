#include "SysTimer.h"
#include <thread>

SysTimer::SysTimer()
{
    mTimer = CreateWaitableTimerEx(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);;
    SetFramerate(60);
}

SysTimer::~SysTimer()
{
    CloseHandle(mTimer);
}

float SysTimer::DeltaTime()
{
    return mDeltaTime;
}

INT64 SysTimer::GetTime()
{
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;

    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&frequency);

    return counter.QuadPart * (INT64(1e9) / frequency.QuadPart);
}

void SysTimer::SetFramerate(int framerate)
{
    if (framerate > 0)
        mInterval = INT64(1e9) / framerate;
    else
        mInterval = 0;

    Sync();
}

INT64 SysTimer::GetInterval()
{
    return mInterval;
}

void SysTimer::Sync()
{
    mLastTime = GetTime();
    mTotalTime = mLastTime;
}

void SysTimer::Tick()
{
    mTotalTime += mInterval;

    while (true)
    {
        INT64 remaining = mTotalTime - GetTime();

        if (remaining <= 2000000) {
            while (GetTime() < mTotalTime)
                std::this_thread::yield();
            break;
        }

        INT64 hus = (remaining - 1000000) / 100;

        LARGE_INTEGER due;
        due.QuadPart = -hus; // Unit of SetWaitableTimer is 100ns. And the value should be negative.

        if (SetWaitableTimerEx(mTimer, &due, 0, NULL, NULL, NULL, 0))
            WaitForSingleObject(mTimer, INFINITE);
    }

    INT64 now = GetTime();
    mDeltaTime = float((now - mLastTime) / 1e9);
    mLastTime = now;
}
