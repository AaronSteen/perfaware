// Function which estimates CPU frequency
//
// Output should look like this:
//      Input size: 1125484600
//      Pair count: 10000000
//      Haversine sum: 4519.9968985727073232
//
//      Total time: 9096.2338ms (CPU freq 4200224880)
//          Startup: 42 (0.00%)
//          Read: 1247842044 (3.27%)
//          MiscSetup: 86118 (0.00%)
//          Parse: 35064925236 (91.78%)
//          Sum: 1614093308 (4.22%)
//          MiscOutput: 279280471 (0.73%)
//
//  Go one by one and see how we get each metric:
//
//      Total time: 9096.2338ms
//          Peformance frequency is going to be 10MHz = 10,000,000 ticks per second
//
//          10,000,000 ticks
//          ---------------    x        what       = 9096.2338ms?
//              1 second
//          
#define _CRT_SECURE_NO_WARNINGS

#include <sys/stat.h>
#include "common.h"
#include "json_parser.c"
#include "platform_metrics.c"

struct buffer 
ReadEntireFile(char *FileName)
{
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
    u64 Prof_Begin = ReadCpuTimer();
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

    u64 Prof_Read = ReadCpuTimer();
    OpenJson(&Json);
    InitInputJson(&Json);

    u64 Prof_MiscSetup = ReadCpuTimer();
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
    u64 Prof_Parse = ReadCpuTimer();
    u64 JsonPairCount = ParseHaversinePairs(&HaversinePairs, &Json, MaxPairCount);

    // Compute
    u64 Prof_Sum = ReadCpuTimer();
    f64 JsonSum = SumHaversineDistances(JsonPairCount, &HaversinePairs);
    f64 JsonAverage = JsonSum / (f64)JsonPairCount;

    u64 Prof_MiscOutput = ReadCpuTimer();
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
            OutputErrorMessage("Error: Number of pairs found in json, %ld, did not match number of answers, %ld, in answers file", JsonPairCount, NumAnswers);
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

    u64 Prof_End = ReadCpuTimer();

    if(HaversinePairs.Data) free((void *)HaversinePairs.Data);
    if(Json.JsonToParse.Data) free((void *)Json.JsonToParse.Data);

    u64 CpuFreq = GuessCpuFreq();

    u64 StartupElapsed = Prof_Read - Prof_Begin;
    u64 ReadElapsed = Prof_MiscSetup - Prof_Read;
    u64 MiscSetupElapsed = Prof_Parse - Prof_MiscSetup;
    u64 ParseElapsed = Prof_Sum - Prof_Parse;
    u64 SumElapsed = Prof_MiscOutput - Prof_Sum;
    u64 MiscOutputElapsed = Prof_End - Prof_MiscOutput;
    u64 TotalCpuElapsed = Prof_End - Prof_Begin;
    
    f64 TotalTime = (f64)TotalCpuElapsed / (f64)CpuFreq;
    f64 StartupRatio = (f64)StartupElapsed / (f64)TotalCpuElapsed;
    f64 ReadRatio = (f64)ReadElapsed / (f64)TotalCpuElapsed;
    f64 MiscSetupRatio = (f64)MiscSetupElapsed / (f64)TotalCpuElapsed;
    f64 ParseRatio = (f64)ParseElapsed / (f64)TotalCpuElapsed;
    f64 SumRatio = (f64)SumElapsed / (f64)TotalCpuElapsed;
    f64 MiscOutputRatio = (f64)MiscOutputElapsed / (f64)TotalCpuElapsed;

    DebugOutput("Total time: %.2fms (CPU freq %llu)", TotalTime * 1000, CpuFreq);
    DebugOutput("    Startup: %llu, (%.2f%)", StartupElapsed, StartupRatio * 100);
    DebugOutput("    Read: %llu, (%.2f%)", ReadElapsed, ReadRatio * 100);
    DebugOutput("    MiscSetup: %llu, (%.2f%)", MiscSetupElapsed, MiscSetupRatio * 100);
    DebugOutput("    Parse: %llu, (%.2f%)", ParseElapsed, ParseRatio * 100);
    DebugOutput("    Sum: %llu, (%.2f%)", SumElapsed, SumRatio * 100);
    DebugOutput("    MiscOutput: %llu, (%.2f%)", MiscOutputElapsed, MiscOutputRatio * 100);
    DebugOutput("\n\n");

    return(0);
}
