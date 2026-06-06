#include "SysTimer.h"
#include <thread>

SysTimer::SysTimer()
{
    m_Timer = CreateWaitableTimerEx(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);;
    SetFramerate(60);
}

float SysTimer::DeltaTime()
{
    return m_DeltaTime;
}

UINT64 SysTimer::GetTime()
{
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;

    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&frequency);

    return counter.QuadPart * (UINT64(1e9) / frequency.QuadPart);
}

void SysTimer::SetFramerate(int framerate)
{
    if (framerate > 0)
        m_Interval = __int64(1e9) / framerate;
    else
        m_Interval = 0;

    Reset();
}

void SysTimer::Reset()
{
    m_LastTime = GetTime();
    m_TotalTime = m_LastTime;
}

void SysTimer::Tick()
{
    m_TotalTime += m_Interval;
    while (true)
    {
        UINT64 remaining = 0;
        UINT64 now = GetTime();

        if (m_TotalTime > now)
            remaining = m_TotalTime - now;

        if (remaining <= 2000000) {
            while (GetTime() < m_TotalTime)
                std::this_thread::yield();
            break;
        }

        INT64 hus = (remaining - 1000000) / 100;

        LARGE_INTEGER due;
        due.QuadPart = -hus; // Unit of SetWaitableTimer is 100ns. And the value should be negative.

        if (SetWaitableTimerEx(m_Timer, &due, 0, NULL, NULL, NULL, 0))
            WaitForSingleObject(m_Timer, INFINITE);
    }

    INT64 now = GetTime();
    m_DeltaTime = float((now - m_LastTime) / 1e9);
    m_LastTime = now;
}
