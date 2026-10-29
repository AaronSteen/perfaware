#define MAX_ANCHORS 4096

#ifndef PROFILER
#define PROFILER 0
#endif

#ifndef READ_BLOCK_TIMER
#define READ_BLOCK_TIMER ReadCpuTimer
#endif

#if PROFILER

struct profile_anchor
{
    u64 TscElapsedExclusive;
    u64 TscElapsedInclusive;
    u64 HitCount;
    u64 ProcessedByteCount;
    const char *Label;
};
profile_anchor gProfileAnchors[MAX_ANCHORS];
static u32 gProfilerParent;

struct profile_block
{
    char const *Label;
    u64 OldTscElapsedInclusive;
    u64 StartTsc;
    u32 ParentIdx;
    u32 AnchorIdx;

    profile_block(char const *Label_, u32 AnchorIdx_, u64 ByteCount)
    {
        ParentIdx = gProfilerParent;

        AnchorIdx = AnchorIdx_;
        Label = Label_;

        profile_anchor *pAnchor = gProfileAnchors + AnchorIdx;
        OldTscElapsedInclusive = pAnchor->TscElapsedInclusive;
        pAnchor->ProcessedByteCount += ByteCount;

        gProfilerParent = AnchorIdx;
        StartTsc = READ_BLOCK_TIMER();
    }

    ~profile_block(void)
    {
        u64 Elapsed = READ_BLOCK_TIMER() - StartTsc;
        gProfilerParent = ParentIdx;

        profile_anchor *pParent = gProfileAnchors + ParentIdx;
        profile_anchor *pAnchor = gProfileAnchors + AnchorIdx;

        pParent->TscElapsedExclusive -= Elapsed;
        pAnchor->TscElapsedExclusive += Elapsed;
        pAnchor->TscElapsedInclusive = OldTscElapsedInclusive + Elapsed;
        ++pAnchor->HitCount;

        pAnchor->Label = Label;
    }
};

#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBandwidth(Name, ByteCount) profile_block NameConcat(Block, __LINE__)(Name, __COUNTER__ + 1, ByteCount)
#define TimeBlock(Name) TimeBandwidth(Name, 0)
#define ProfilerEndOfCompilationUnit static_assert(__COUNTER__ < ArrayCount(gProfileAnchors), "Number of profile points exceeds size of profiler::Anchors array")

// Will expand to, e.g.
//     profile_block Block176(__func__, 2 + 1);; (TimeFunction)
//     profile_block Block192("Lookup and Convert", 3 + 1);; (TimeBlock)
//
//      Block is stringized by NameConcat with the line number to produce Block176
//      The parentheses following are literal parentheses from this: 
//          (Name, __COUNTER__ + 1)
//      Name becomes either __func__ in the case of TimeFunction, (which is copied as the label in the the anchor struct),
//          or whatever string is passed in. 
//      Then, when the constructor below runs, __COUNTER__ + 1 becomes the index for writing into gProfiler->Anchors,
//          current gProfilerParent is temporarily stored, and replaced by AnchorIdx while while the block is running,
//          since it is the current scope.
//      profile_block(char const *Label_, u32 AnchorIdx_)
//      {
//          ParentIdx = gProfilerParent;
//          AnchorIdx = AnchorIdx_;
//          Label = Label_;
//          gProfilerParent = AnchorIdx;
//          StartTsc = ReadCpuTimer();
//      }
//      Finally, when the profile_block struct is destroyed:
//      ~profile_block(void)
//      {
//          u64 Elapsed = ReadCpuTimer() - StartTsc;
//          gProfilerParent = ParentIdx;
//          profile_anchor *pParent = gProfileAnchors + ParentIdx;
//          profile_anchor *pAnchor = gProfileAnchors + AnchorIdx;
//          pParent->TscElapsedChildren += Elapsed;
//          pAnchor->TscElapsed += Elapsed;
//          ++pAnchor->HitCount;
//          pAnchor->Label = Label;
//      }
//
//      gProfilerParent is restored to ParentIdx, and pParent's Elapsed children is updated.

static void
PrintTimeElapsed(u64 TotalElapsed, u64 TimerFreq, profile_anchor *pAnchor)
{
    char Temp[MAX_STRING_LEN];
    f64 ExclusivePercent = 100.0 * ((f64)pAnchor->TscElapsedExclusive / (f64)TotalElapsed);
    snprintf(Temp, MAX_STRING_LEN, "%s[%llu]: %llu (%.2f%%", pAnchor->Label, pAnchor->HitCount, pAnchor->TscElapsedExclusive, ExclusivePercent);
    OutputDebugStringA(Temp);
    if(pAnchor->TscElapsedInclusive != pAnchor->TscElapsedExclusive) {
        f64 InclusivePercent = 100.0 * ((f64)pAnchor->TscElapsedInclusive / TotalElapsed);
        snprintf(Temp, MAX_STRING_LEN, ", %.2f%% w/ children", InclusivePercent);
        OutputDebugStringA(Temp);
    }
    OutputDebugStringA(")");

    if (pAnchor->ProcessedByteCount) {
        f64 OneMb = 1024.0 * 1024.0;
        f64 OneGb = OneMb * 1024.0;

        f64 SecondsElapsed = (f64)pAnchor->TscElapsedInclusive / (f64)TimerFreq;
        f64 MbProcessed = (f64)pAnchor->ProcessedByteCount / OneMb;
        f64 GbProcessed = (f64)pAnchor->ProcessedByteCount / OneGb;
        f64 GbPerSec = GbProcessed / SecondsElapsed;

        snprintf(Temp, MAX_STRING_LEN, "  %.3fmb at %.2fgb/s", MbProcessed, GbPerSec);
        OutputDebugStringA(Temp);
    }
    OutputDebugStringA("\n");
}


static void
PrintAnchorData(u64 TotalCpuElapsed, u64 TimerFreq)
{
    for (int AnchorIdx = 1; AnchorIdx < ArrayCount(gProfileAnchors); ++AnchorIdx) {
        profile_anchor *pAnchor = gProfileAnchors + AnchorIdx;
        if (pAnchor->TscElapsedInclusive) {
            PrintTimeElapsed(TotalCpuElapsed, TimerFreq, pAnchor);
        }
    }
}

#else

#define TimeBlock(...)
#define PrintAnchorData(...)
#define ProfilerEndOfCompilationUnit

#endif

struct profiler
{
    u64 StartTsc;
    u64 EndTsc;
};
static profiler gProfiler;

#define TimeFunction TimeBlock(__func__)

static u64 
EstimateBlockTimerFreq(void)
{
    (void)&EstimateCpuTimerFreq;

    u64 MsToWait = 100;
    u64 OsFreq = GetOsTimerFreq();

    u64 BlockStart = READ_BLOCK_TIMER();
    u64 OsStart = ReadOsTimer();
    u64 OsEnd = 0;
    u64 OsElapsed = 0;

    // e.g., Windows OsFreq = 10'000'000; MsToWait = 100:
    //      10'000'000 * 100 / 1'000
    //      Multiplying the OsFreq (how many ticks in a second) by the fraction of a sec (1/10) we want to wait
    u64 OsWaitTime = OsFreq * MsToWait / 1000;
    while (OsElapsed < OsWaitTime) {
        OsEnd = ReadOsTimer();
        OsElapsed = OsEnd - OsStart;
    }

    u64 BlockEnd = READ_BLOCK_TIMER();
    u64 BlockElapsed = BlockEnd - BlockStart;

    u64 BlockFreq = 0;
    if (OsElapsed) {
        BlockFreq = BlockElapsed * OsFreq / OsElapsed;
    }

    return BlockFreq;
}

static void
BeginProfile(void)
{
    gProfiler.StartTsc = READ_BLOCK_TIMER();
}

static void
EndAndPrintProfile(void)
{
    gProfiler.EndTsc = READ_BLOCK_TIMER();
    u64 TimerFreq = EstimateBlockTimerFreq();

    u64 TotalTscElapsed = gProfiler.EndTsc - gProfiler.StartTsc;

    if (TimerFreq) {
        f64 MsElapsed = 1000.0 * (f64)TotalTscElapsed / (f64)TimerFreq;
        DebugOutput("Total time: %.4fms (Timer freq %llu); Total elapsed: %llu", MsElapsed, TimerFreq, TotalTscElapsed);
    }

    PrintAnchorData(TotalTscElapsed, TimerFreq);
}
