#include "common.h"
#include "platform_metrics.c"

int
main(void)
{
    u64 OsFreq = GetOsTimerFreq();
    DebugOutput("\n\n     OS Freq: %llu\n", OsFreq);

    u64 OsStart = ReadOsTimer();
    u64 OsEnd = 0;
    u64 OsElapsed = 0;

    while(OsElapsed < OsFreq)
    {
        OsEnd = ReadOsTimer();
        OsElapsed = OsEnd - OsStart;
    }

    DebugOutput("    OS Timer: %llu -> %llu = %llu elapsed\n", OsStart, OsEnd, OsElapsed);
    DebugOutput("    OS Seconds: %.4f\n", (f64)OsElapsed/(f64)OsFreq);

    return(0);
}
