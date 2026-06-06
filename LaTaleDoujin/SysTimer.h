#pragma once
#include <Windows.h>

class SysTimer
{
public:
    SysTimer();

    float DeltaTime();
    UINT64 GetTime();

    void SetFramerate(int framerate);
    void Reset();
    void Tick();

private:
    HANDLE m_Timer;
    UINT64 m_TotalTime = 0;
    UINT64 m_LastTime = 0;
    UINT64 m_Interval = 0;
    float m_DeltaTime = 0;
};

