#pragma once
#include "repetition.h"

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
PrintTime(char const *pLabel, f64 CpuTime, u64 CpuTimerFreq, u64 ByteCount)
{
    printf("%s: %.0f", pLabel, CpuTime);
    if (CpuTimerFreq) {
        f64 Seconds = SecondsFromCpuTime(CpuTime, CpuTimerFreq);
        printf(" (%fms)", 1000.0f * Seconds);

        if (ByteCount) {
            f64 Gb = (1024.0f * 1024.0f * 1024.0f); 
            f64 BestBandwidth = ByteCount / (Gb * Seconds);
            printf(" %fgb/s", BestBandwidth);
        }
    }
}

static void
PrintResults(repetition_test_results Results, u64 CpuTimerFreq, u64 ByteCount)
{
    PrintTime("Min", (f64)Results.MinTime, CpuTimerFreq, ByteCount);
    printf("\n");

    PrintTime("Max", (f64)Results.MaxTime, CpuTimerFreq, ByteCount);
    printf("\n");

    if (Results.TestCount) {
        PrintTime("Avg", (f64)Results.TotalTime / (f64)Results.TestCount, CpuTimerFreq, ByteCount);
        printf("\n");
    }
}

static void
Error(repetition_tester *pTester, char const *pMessage)
{
    pTester->Mode = TestMode_Error;
    fprintf(stderr, "ERROR: %s", pMessage);
}

static void
NewTestWave(repetition_tester *pTester, u64 TargetProcessedByteCount, u64 CpuTimerFreq, u32 SecondsToTry = 10)
{
    if (pTester->Mode == TestMode_Uninitialized) {
        pTester->Mode = TestMode_Testing;
        pTester->TargetProcessedByteCount = TargetProcessedByteCount;
        pTester->CpuTimerFreq = CpuTimerFreq;
        pTester->PrintNewMinimums = true;
        pTester->Results.MinTime = (u64)-1;
    }
    else if (pTester->Mode == TestMode_Completed) {
        pTester->Mode = TestMode_Testing;
        if (pTester->TargetProcessedByteCount != TargetProcessedByteCount) {
            Error(pTester, "TargetProcessedByteCount changed");
        }
        if (pTester->CpuTimerFreq != CpuTimerFreq) {
            Error(pTester, "Cpu frequency changed");
        }
    }

    pTester->TryForTime = SecondsToTry * CpuTimerFreq;
    pTester->TestsStartedAt = ReadCpuTimer();
}

static void
BeginTime(repetition_tester *pTester)
{
    ++pTester->OpenBlockCount;
    pTester->TimeAccumulatedOnThisTest -= ReadCpuTimer();
}

static void
EndTime(repetition_tester *pTester)
{
    ++pTester->CloseBlockCount;
    pTester->TimeAccumulatedOnThisTest +=ReadCpuTimer();
}

static void
CountBytes(repetition_tester *pTester, u64 ByteCount)
{
    pTester->BytesAccumulatedOnThisTest += ByteCount;
}

static b32
IsTesting(repetition_tester *pTester)
{
    if (pTester->Mode == TestMode_Testing) {
        u64 CurrentTime = ReadCpuTimer();
        if (pTester->OpenBlockCount) {
            if (pTester->OpenBlockCount != pTester->CloseBlockCount) {
                Error(pTester, "Unbalanced BeginTime/EndTime");
            }
            if (pTester->BytesAccumulatedOnThisTest != pTester->TargetProcessedByteCount) {
                Error(pTester, "Processed byte count mismatch");
            }
            if (pTester->Mode == TestMode_Testing) {
                repetition_test_results *pResults = &pTester->Results;
                u64 ElapsedTime = pTester->TimeAccumulatedOnThisTest;
                pResults->TestCount += 1;
                pResults->TotalTime += ElapsedTime;
                if (pResults->MaxTime < ElapsedTime) {
                    pResults->MaxTime = ElapsedTime;
                }
                if (pResults->MinTime > ElapsedTime) {
                    pResults->MinTime = ElapsedTime;
                    pTester->TestsStartedAt = CurrentTime;
                    if (pTester->PrintNewMinimums) {
                        PrintTime("Min", pResults->MinTime, pTester->CpuTimerFreq, pTester->BytesAccumulatedOnThisTest);
                        printf("               \r");
                    }
                }

                 pTester->OpenBlockCount = 0;
                 pTester->CloseBlockCount = 0;
                 pTester->TimeAccumulatedOnThisTest = 0;
                 pTester->BytesAccumulatedOnThisTest = 0;
            }
        }

        if ((CurrentTime - pTester->TestsStartedAt) > pTester->TryForTime) {
            pTester->Mode = TestMode_Completed;
            printf("                                                          \r");
            PrintResults(pTester->Results, pTester->CpuTimerFreq, pTester->TargetProcessedByteCount);
        }
    }

    b32 Result = (pTester->Mode == TestMode_Testing);
    return Result;
}



