#define _CRT_SECURE_NO_WARNINGS
#define MAX_TIMED_SCOPES 256
#define MAX_STRING_LEN 512

#include <sys/stat.h>
#include "common.h"
#include "json_parser.c"
#include "platform_metrics.c"

struct timed_scope
{
    const char *Label;
    u64 Start;
    u64 End;
};

static timed_scope gEntire;
static timed_scope gTimedScopes[MAX_TIMED_SCOPES * 2];
static int gIdx;

struct timer
{
    u64 Start;
    char const *Label;
    int Idx;

    timer(char const *Label) : Start(ReadCpuTimer()), Label(Label), Idx(gIdx++)
    {
    }

    ~timer() 
    {
        gTimedScopes[Idx] = {Label, Start, ReadCpuTimer()};
    }
};

static u64
GetElapsed(u64 Start, u64 End)
{
    Assert(Start > 0);
    Assert(End > 0);
    Assert(End > Start);
    u64 Result = End - Start;
    return(Result);
}

static void
BeginProfile()
{
    gEntire = {"Entire", ReadCpuTimer(), 0};
}

static void
EndAndPrintProfile(void)
{
    gEntire.End = ReadCpuTimer();
    if (gIdx > MAX_TIMED_SCOPES) {
        DebugOutput("Error: Max timed scopes was %d, but recorded %d. Aborting.", MAX_TIMED_SCOPES, gIdx);
    }
    else {
        u64 CpuFreq = GuessCpuFreq();
        u64 TotalElapsed = GetElapsed(gEntire.Start, gEntire.End);
        f64 MsElapsed = (f64)TotalElapsed / CpuFreq * 1000.0;
        DebugOutput("Total time: %.4fms (Cpu freq %llu)", MsElapsed, CpuFreq);
        for (int NthScope = 0; NthScope < gIdx; ++NthScope) {
            timed_scope *pScope = &gTimedScopes[NthScope];
            Assert(pScope->Start > gEntire.Start);
            Assert(pScope->End < gEntire.End);
            u64 NthScopeElapsed = GetElapsed(pScope->Start, pScope->End);
            DebugOutput("    %s: %llu (%.2f%%)", pScope->Label, NthScopeElapsed, (f64)NthScopeElapsed / (f64)TotalElapsed * 100.0);
        }
    }
}

#define TimeFunction timer Timer(__func__)
#define TimeBlock(Label) timer Timer(Label)

struct buffer 
ReadEntireFile(char *FileName)
{
    TimeFunction;
    // note we copied this from casey; he uses FileName here but
    //      we use Filename (no capped "n") in our main function below
    struct buffer Result = {};

    FILE *File = fopen(FileName, "rb");
    if(File)
    {
#if _WIN32
        struct __stat64 Stat;
        _stat64(FileName, &Stat);
#else
        struct stat Stat;
        stat(FileName, &Stat);
#endif

        Result = AllocateBuffer(Stat.st_size);
        if(Result.Data)
        {
            if(fread(Result.Data, Result.NumBytes, 1, File) != 1)
            {
                OutputErrorMessage("ERROR: Unable to read \"%s\".\n", FileName);
                exit(1);
            }
        }
        fclose(File);
    }
    else
    {
        OutputErrorMessage("ERROR: Unable to open \"%s\".\n", FileName);
        exit(1);
    }

    return Result;
}

static void
InitInputJson(struct json *Json)
{
    if(Json->JsonToParse.NumBytes > GIGABYTES(2))
    {
        OutputErrorMessage("This utility only parses .json files of size %llu or smaller. You supplied a file of size %llu", GIGABYTES(2), Json->JsonToParse.NumBytes);
        exit(1);
    }

    Json->JsonToParse.Data = (u8 *)malloc(Json->JsonToParse.NumBytes);
    if(Json->JsonToParse.Data == NULL)
    {
        OutputErrorMessage("malloc for input json failed");
        exit(1);
    }
    size_t BytesRead = fread(Json->JsonToParse.Data, sizeof(u8), Json->JsonToParse.NumBytes, Json->FilePointer);
    if(BytesRead != Json->JsonToParse.NumBytes)
    {
        OutputErrorMessage("Error when reading json into buffer");
        exit(1);
    }
    fclose(Json->FilePointer);
}

static u64
ParseHaversinePairs(struct buffer *HaversinePairs, struct json *Json, u64 MaxPairCount)
{
    TimeFunction;
    // Definitions:
    //      ParsedJson: The linked list created by the parser. Stores elements as nodes
    //          containing key, value, FirstSubElement and NextSibling.
    //
    //      ParsedJsonPairsStart: The node containing the data for the first Haversine pair we
    //          found in the Json. We traverse this, extract the data, and store it in the HaversinePairs buffer.
    //
    //      HaversinePairs: Buffer containing the HaversinePairs we extracted. We iterate over this
    //          to perform the actual Haversine computations.
    struct json_element *ParsedJson = ParseJson(Json);
    struct json_element *ParsedJsonPairsStart = LookUpJsonElement(ParsedJson, CONSTANT_STRING("pairs"));

    u64 PairCount = 0;
    if(ParsedJsonPairsStart)
    {
        TimeBlock("Lookup and Convert");
        for(struct json_element *Element = ParsedJsonPairsStart->FirstSubElement;
            Element && (PairCount < MaxPairCount);
            Element = Element->NextSibling)
        {
            struct haversine_pair *Pair = (struct haversine_pair *)HaversinePairs->Data + PairCount++;

            Pair->X0 = ConvertJsonStringToF64(Element, CONSTANT_STRING("x0"));
            Pair->Y0 = ConvertJsonStringToF64(Element, CONSTANT_STRING("y0"));
            Pair->X1 = ConvertJsonStringToF64(Element, CONSTANT_STRING("x1"));
            Pair->Y1 = ConvertJsonStringToF64(Element, CONSTANT_STRING("y1"));

        }
    }

    FreeJson(ParsedJson);

    return(PairCount);
}

static f64
SumHaversineDistances(u64 PairCount, struct buffer *HaversinePairsBuffer)
{
    TimeFunction;

    struct haversine_pair *Pairs = (struct haversine_pair *)HaversinePairsBuffer->Data;
    f64 Result = 0;
    for(u64 PairIdx = 0; 
        PairIdx < PairCount;
        ++PairIdx)
    {
        struct haversine_pair Pair = Pairs[PairIdx];
        f64 Computation = Haversine(Pair);
        Result += Computation;
    }

    return(Result);
}

int
main(int ArgCount, char **ArgVector)
{
    BeginProfile();
    // Check usage
    if(!((ArgCount == 2) || (ArgCount == 3)))
    {
        OutputErrorMessage("Either two or three arguments required, but %d was/were supplied", ArgCount);
        exit(1);
    }

    char *Filename = ArgVector[1];
    char *Extension = GetFileExtension(Filename);
    if(strcmp(Extension, ".json") != 0)
    {
        OutputErrorMessage("This utility only parses .json files. You supplied a file with extension %s", Extension);
        exit(1);
    }

    // Init json
    struct json Json;
    Json.Filename = Filename;

    OpenJson(&Json);
    InitInputJson(&Json);

    // Validate json
    u32 MinBytesForJsonPair = 6 * 4;
    u64 MaxPairCount = Json.JsonToParse.NumBytes / MinBytesForJsonPair;
    if(MaxPairCount == 0)
    {
        OutputErrorMessage("Input json file contained no data");
        exit(1);
    }

    // Alloc pairs memory
    struct buffer HaversinePairs = AllocateBuffer(sizeof(struct haversine_pair) * MaxPairCount);

    // Parse
    u64 JsonPairCount = ParseHaversinePairs(&HaversinePairs, &Json, MaxPairCount);

    // Compute
    f64 JsonSum = SumHaversineDistances(JsonPairCount, &HaversinePairs);
    f64 JsonAverage = JsonSum / (f64)JsonPairCount;

    DebugOutput("\n\n");
    DebugOutput("Input size: %llu", Json.JsonToParse.NumBytes);
    DebugOutput("Pair count: %llu", JsonPairCount);
    DebugOutput("Haversine sum: %.16f", JsonSum);

    // Validate computation
    DebugOutput("Avg. Haversine for json pairs: %.16f\n", JsonAverage);
    if(ArgCount == 3)
    {
        char *AnswersFilename = ArgVector[2];
        char *AnswersExtension = GetFileExtension(AnswersFilename);
        if(strcmp(AnswersExtension, ".f64") != 0)
        {
            OutputErrorMessage("Need .f64 file for second argument. You provided file with extension %s", AnswersExtension);
            exit(1);
        }

        struct buffer Answers = ReadEntireFile(AnswersFilename);

        // The final 8 bytes in the file comprise the computed average of the preceding f64s in the file. So don't
        //      take these last 8 bytes in the file into account when computing the number of answers in the file.
        u64 NumAnswers = (Answers.NumBytes - sizeof(f64)) / sizeof(f64);
        if(NumAnswers != JsonPairCount)
        {
            OutputErrorMessage("Error: Number of pairs found in json, %llu, did not match number of answers, %llu, in answers file", JsonPairCount, NumAnswers);
            exit(1);
        }

        f64 *AnswerData = (f64 *)Answers.Data;
        f64 AnswersAverage = AnswerData[NumAnswers]; // NumAnswers is the total number of f64s in the file less 1,
                                                     //         so using it to index into a 0-indexed file will point to the last f64 in the file
        DebugOutput("\nValidation");
        DebugOutput("==============");
        DebugOutput("Reference average: %.16f", AnswersAverage);
        DebugOutput("Difference: %.16f", JsonAverage - AnswersAverage);
        DebugOutput("\n\n");
    }

    if(HaversinePairs.Data) free((void *)HaversinePairs.Data);
    if(Json.JsonToParse.Data) free((void *)Json.JsonToParse.Data);

    EndAndPrintProfile();
    return(0);

}
