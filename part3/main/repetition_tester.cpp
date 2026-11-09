enum test_mode : u32 
{
    TestMode_Uninitialized,
    TestMode_Testing,
    TestMode_Completed,
    TestMode_Error,
};

struct repetition_test_results 
{
    u64 TestCount;
    u64 TotalTime;
    u64 MaxTime;
    u64 MinTime;
};

struct repetition_tester
{
    u64 TargetProcessedByteCount;
    u64 CpuTimerFreq;
    u64 TryForTime;
    u64 TestsStartedAt;

    test_mode Mode;
    b32 PrintNewMinimums;
    u32 OpenBlockCount;
    u32 CloseBlockCount;
    u64 TimeAccumulatedOnThisTest;
    u64 BytesAccumulatedOnThisTest;

    repetition_test_results Results;
};

static f64
SecondsFromCpuTime(f64 CpuTime, u64 CpuTimerFreq)
{
    f64 Result = 0.0;
    if (CpuTimerFreq) {
        Result = (CpuTime / (f64)CpuTimerFreq);
    }

    return Result;
}

static void
PrintTime(char const *Label, f64 CpuTime, u64 CpuTimerFreq, u64 ByteCount)
{
#define TEMP_BUF_LEFT (ArrayCount(Temp, MAX_STRING_LEN + 1) - strnlen(Temp, MAX_STRING_LEN)
    char Temp[MAX_STRING_LEN + 1];

    snprintf(Temp, TEMP_BUF_LEFT, "%s: %.0f", Label, CpuTime);
    if (CpuTimerFreq) {
        f64 Seconds = SecondsFromCpuTime(CpuTime, CpuTimerFreq);
        DebugOutput(" (%fms)", 1000.0f*Seconds);

        if (ByteCount) {
            f64 Gb = (1024.0f * 1024.0f * 1024.0f);
            f64 BestBandwidth = ByteCount / (Gb * Seconds);
            snprintf(Temp, TEMP_BUF_LEFT, " %fgb/s", BestBandwidth);
        }
    }
#undef TEMP_BUF_LEFT
}



