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
    void Sync();
    void Tick();

private:
    HANDLE mTimer;
    INT64 mTotalTime = 0;
    INT64 mLastTime = 0;
    INT64 mInterval = 0;
    float mDeltaTime = 0;
};

