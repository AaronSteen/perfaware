#define MAX_ANCHORS 4096

#ifndef PROFILER
#define PROFILER 0
#endif

#if PROFILER

struct profile_anchor
{
    u64 TscElapsedExclusive;
    u64 TscElapsedInclusive;
    u64 HitCount;
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

    profile_block(char const *Label_, u32 AnchorIdx_)
    {
        ParentIdx = gProfilerParent;
        AnchorIdx = AnchorIdx_;
        Label = Label_;
        gProfilerParent = AnchorIdx;
        OldTscElapsedInclusive = gProfileAnchors[AnchorIdx].TscElapsedInclusive;
        StartTsc = ReadCpuTimer();
    }

    ~profile_block(void)
    {
        u64 Elapsed = ReadCpuTimer() - StartTsc;
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
#define TimeBlock(Name) profile_block NameConcat(Block, __LINE__)(Name, __COUNTER__ + 1);
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
PrintTimeElapsed(u64 TotalElapsed, profile_anchor *pAnchor)
{
    f64 ExclusivePercent = 100.0 * (f64)pAnchor->TscElapsedExclusive / (f64)TotalElapsed;
    if (pAnchor->TscElapsedExclusive != pAnchor->TscElapsedInclusive) {
        f64 InclusivePercent = 100.0 * (f64)pAnchor->TscElapsedInclusive / (f64)TotalElapsed;
        DebugOutput("   %s[%llu]: %llu (%.2f%%, %.2f%% with children)", pAnchor->Label, pAnchor->HitCount, pAnchor->TscElapsedExclusive, ExclusivePercent, InclusivePercent);
    }
    else {
        DebugOutput("   %s[%llu]: %llu (%.2f%%)", pAnchor->Label, pAnchor->HitCount, pAnchor->TscElapsedExclusive, ExclusivePercent);
    }
}


static void
PrintAnchorData(u64 TotalCpuElapsed)
{
    for (int AnchorIdx = 1; AnchorIdx < ArrayCount(gProfileAnchors); ++AnchorIdx) {
        profile_anchor *pAnchor = gProfileAnchors + AnchorIdx;
        if (pAnchor->TscElapsedInclusive) {
            PrintTimeElapsed(TotalCpuElapsed, pAnchor);
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

static void
BeginProfile(void)
{
    gProfiler.StartTsc = ReadCpuTimer();
}

static void
EndAndPrintProfile(void)
{
    gProfiler.EndTsc = ReadCpuTimer();
    u64 CpuFreq = GuessCpuFreq();

    u64 TotalCpuElapsed = gProfiler.EndTsc - gProfiler.StartTsc;

    if (CpuFreq) {
        f64 MsElapsed = 1000.0 * (f64)TotalCpuElapsed / (f64)CpuFreq;
        DebugOutput("Total time: %.4fms (Cpu freq %llu); Total elapsed: %llu", MsElapsed, CpuFreq, TotalCpuElapsed);
    }

    PrintAnchorData(TotalCpuElapsed);
}
