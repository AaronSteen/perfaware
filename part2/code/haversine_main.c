#include "common.h"
#include "json_parser.c"


void
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
}

u64
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

f64
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
    if(ArgCount != 2)
    {
        OutputErrorMessage("Two arguments required, but %d was/were supplied", ArgCount);
        exit(1);
    }

    char *Filename = ArgVector[1];
    char *Extension = GetFileExtension(Filename);
    if(strcmp(Extension, ".json") != 0)
    {
        OutputErrorMessage("This utility only parses .json files. You supplied a file with extension %s", Extension);
        exit(1);
    }

    struct json Json;
    Json.Filename = Filename;
    OpenJson(&Json);
    InitInputJson(&Json);

    u32 MinBytesForJsonPair = 6 * 4;
    u64 MaxPairCount = Json.JsonToParse.NumBytes / MinBytesForJsonPair;
    if(MaxPairCount == 0)
    {
        OutputErrorMessage("Input json file contained no data");
        exit(1);
    }

    struct buffer HaversinePairs = AllocateBuffer(sizeof(struct haversine_pair) * MaxPairCount);
    u64 PairCount = ParseHaversinePairs(&HaversinePairs, &Json, MaxPairCount);
    f64 Sum = SumHaversineDistances(PairCount, &HaversinePairs);
    f64 Average = Sum / (f64)PairCount;

    char PrintBuffer[256];
    sprintf(PrintBuffer, "Expected average: %f\n", Average);
    OutputDebugStringA(PrintBuffer);

    if(Json.FilePointer) fclose(Json.FilePointer);
    if(Json.JsonToParse.Data) free((void *)Json.JsonToParse.Data);

    // A json input file containing 10 million pairs is 1,085,542,039 bytes, or a little more than 1 GB.
    // Therefore let's say that we support input json file sizes of no more than 2GB.

    return(0);
}
