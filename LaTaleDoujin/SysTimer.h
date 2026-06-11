#pragma once
#include <Windows.h>

class SysTimer
{
public:
    SysTimer();
    ~SysTimer();

    float DeltaTime();
    INT64 GetTime();

    void SetFramerate(int framerate);
    void Reset();
    void Tick();

private:
    HANDLE m_Timer;
    INT64 m_TotalTime = 0;
    INT64 m_LastTime = 0;
    INT64 m_Interval = 0;
    float m_DeltaTime = 0;
};

