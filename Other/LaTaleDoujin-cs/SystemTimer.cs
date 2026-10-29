using System.Runtime.InteropServices;
using System.Threading;
using Windows.Win32.Foundation;
using static Windows.Win32.PInvoke;

namespace MyDXLibrary;

public class SystemTimer
{
    public int Framerate
    {
        get;
        set
        {
            field = value;
            Interval = value > 0 ? ((long)1e9) / value : 0;
            Sync();
        }
    } 

    public long Time
    {
        get
        {
            QueryPerformanceCounter(out var counter);
            QueryPerformanceFrequency(out var freqency);
            return counter * ((long)1e9) / freqency;
        }
    }

    public float DeltaTime { get; private set; }
    public long Interval { get; private set; }

    private SafeHandle _timer;
    private long _total;
    private long _last;

    public SystemTimer()
    {
        _timer = CreateWaitableTimerEx(null, null, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, 0x1F0003 /*TIMER_ALL_ACCESS*/);
        Framerate = 60;
    }

    ~SystemTimer()
    {
        _timer.Close();
    }

    public void Sync()
    {
        _last = Time;
        _total = _last;
    }

    public unsafe void Tick()
    {
        _total += Interval;

        while (true)
        {
            long remaining = _total - Time;

            if (remaining <= 2000000) {
                while (Time < _total)
                    Thread.Yield();
                break;
            }

            long hus = (remaining - 1000000) / 100;

            if (SetWaitableTimerEx(_timer, -hus, 0, null, null, null, 0))
                WaitForSingleObject(_timer, INFINITE);
        }

        long now = Time;
        DeltaTime = (float)((now - _last) / 1e9);
        _last = now;
    }
}
