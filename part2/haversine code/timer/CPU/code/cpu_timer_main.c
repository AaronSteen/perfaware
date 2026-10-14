#include "common.h"
#include "platform_metrics.c"

// note: this coresponds to listing_0073_cpu_timer_guessfreq_main.cpp, not listing_0072_cpu_timer_main.cpp
// what is the purpose of this?
//      since we know how many ticks performance counter has per second, we can use this to
//      confirm that even when measuring at sub-second intervals, rdtsc remains accurate.
//
//      e.g., processor speed is 2.3GHz, or 2,300,000,000 cycles per second.
//      over 1 second, we would expect CpuEnd - CpuStart to equal approx 2,300,000,000.
//
//      over 100 milliseconds, or 1/10 of a second, we would expect CpuEnd - CpuStart to equal approximately 230,000,000
//
//      over 10 milliseconds, or 1/100 of a sec, we would expect approx 23,000,000
//
//      over 1 millisecond, or 1/1000 of a sec, we would expect approx 2,300,000.
//
//      And this takes these measurements then extrapolates them to 1 second. if rdtsc is accurate at sub-second
//              intervals, the normalized measurement will be very close to 2,300,000,000
//
//      On a 2.3Ghz machine, normalized cycles/second measured at different intervals was:
//          1000 ms (1 second):         2304001780
//          100  ms (1/10 second):      2304024660 
//          10   ms (1/100 second):     2304243600
//          1    ms (1/1000 second):    2306306000
//

int
main(int ArgCount, char *ArgVector[])
{
    u64 MsToWait = 1000;
    if(ArgCount == 2)
    {
        MsToWait = atoll(ArgVector[1]);
    }

    u64 OsFreq = GetOsTimerFreq();
    printf("    OS Freq: %llu\n", OsFreq);

    u64 CpuStart = ReadCpuTimer();
    u64 OsStart = ReadOsTimer();
    u64 OsEnd = 0;
    u64 OsElapsed = 0;
    u64 OsWaitTime = OsFreq * MsToWait / 1000; // MsToWait / 1000 gives what % of a second we want to wait for
    while(OsElapsed < OsWaitTime)
    {
        OsEnd = ReadOsTimer();
        OsElapsed = OsEnd - OsStart;
    }

    u64 CpuEnd = ReadCpuTimer();
    u64 CpuElapsed = CpuEnd - CpuStart;
    u64 CpuFreq = 0;
    if(OsElapsed)
    {
        CpuFreq = CpuElapsed * OsFreq / OsElapsed; // OsFreq / OsElapsed is a ratio representing what % larger/smaller the time we waited for was
                                                   //       when compared with 1 second.
                                                   //       The ratio tells us how much we need to multiply CpuElapsed
                                                   //       by to extrapolate it to one second.
    }

    printf("   OS Timer: %llu -> %llu = %llu elapsed\n", OsStart, OsEnd, OsElapsed);
    printf(" OS Seconds: %.4f\n", (f64)OsElapsed/(f64)OsFreq);

    printf("  CPU Timer: %llu -> %llu = %llu elapsed\n", CpuStart, CpuEnd, CpuElapsed);
    printf("   CPU Freq: %llu (guessed)\n", CpuFreq);

    return(0);
}
