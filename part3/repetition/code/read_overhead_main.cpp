#include "repetition.h"

struct test_function
{
    char const *Name;
    read_overhead_test_func *Func;
};

test_function TestFunctions[] = {
    {"fread", ReadViaFRead},
    {"_read", ReadViaRead},
    {"ReadFile", ReadViaReadFile},
};

int
main(int ArgC, char **ArgV)
{
    // reference pointers to prevent compiler from complaining about unused functions
    //      ... but i think we compile already with no unused function turned off?
    // (void)&IsInBounds;
    // (void)&AreEqual;

    u64 CpuTimerFreq = EstimateCpuTimerFreq();
    
    if (ArgC == 2) {
        char *pFilename = ArgV[1];
        struct __stat64 Stat;
        _stat64(pFilename, &Stat);

        read_parameters Params = {};
        Params.Dest = AllocateBuffer(Stat.st_size);
        Params.pFilename = pFilename;

        if (Params.Dest.Count > 0) {
            repetition_tester Testers[ArrayCount(TestFunctions)][AllocType_Count] = {};
            
            for(;;) {
                for (u32 FuncIdx = 0; FuncIdx < ArrayCount(TestFunctions); ++FuncIdx) {
                    for (u32 AllocType = 0; AllocType < AllocType_Count; ++AllocType) {
                        Params.AllocType = (allocation_type)AllocType;
                        
                        repetition_tester *pTester = &Testers[FuncIdx][AllocType];
                        test_function TestFunc = TestFunctions[FuncIdx];
                        
                        printf("\n--- %s%s%s ---\n",
                               DescribeAllocationType(Params.AllocType),
                               Params.AllocType ? " + " : "",
                               TestFunc.Name);
                        NewTestWave(pTester, Params.Dest.Count, CpuTimerFreq);
                        TestFunc.Func(pTester, &Params);
                    }
                }
            }
        }
        else {
            printf("ERROR: Test data size must be non-zero\n");
        }
    }
    else {
        printf("Usage: %s [existing filename]", ArgV[0]);
    }

    return 0;
}
