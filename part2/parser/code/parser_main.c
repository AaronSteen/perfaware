#include "common.h"
#include "functions.c"
#include "json_parser.c"

int
main(int ArgCount, char **ArgVector)
{
    if(ArgCount != 2)
    {
        OutputErrorMessage("Two arguments required, but %d was/were supplied", ArgCount);
        exit(1);
    }

    struct json Json = {0};
    Json.Filename = ArgVector[1];
    char *Extension = GetFileExtension(Json.Filename);
    if(strcmp(Extension, ".json") != 0)
    {
        OutputErrorMessage("This utility only parses .json files. You supplied a file with extension %s", Extension);
        exit(1);
    }

    OpenJson(&Json);
    if(Json.JsonToParse.NumBytes > GIGABYTES(2))
    {
        OutputErrorMessage("This utility only parses .json files of size %llu or smaller. You supplied a file of size %llu", GIGABYTES(2), Json.JsonToParse.NumBytes);
        exit(1);
    }

    Json.JsonToParse.Data = (u8 *)malloc(Json.JsonToParse.NumBytes);
    if(Json.JsonToParse.Data == NULL)
    {
        OutputErrorMessage("malloc failed");
        exit(1);
    }
    size_t BytesRead = fread(Json.JsonToParse.Data, sizeof(u8), Json.JsonToParse.NumBytes, Json.FilePointer);
    if(BytesRead != Json.JsonToParse.NumBytes)
    {
        OutputErrorMessage("Error when reading json into buffer");
        exit(1);
    }

    struct json_parser Parser;
    Parser.JsonToParse = Json.JsonToParse;
    Parser.HadError = false;
    Parser.Cursor = 0;

    struct buffer NeedEmptyBufferToGetThisGoing = {};
    struct json_element *ParsedJson = ParseJsonElement(&Parser, NeedEmptyBufferToGetThisGoing, GetJsonToken(&Parser));

    // this 1000 is temp value, need to compute based on input
    u64 MaxPairCount = 1000;
    struct buffer HaversinePairsStart = AllocateBuffer(sizeof(struct haversine_pair) * MaxPairCount);
    u64 PairCount = 0;
    struct json_element *ParsedPairsArrayStart = LookUpJsonElement(ParsedJson, CONSTANT_STRING("pairs"));
    if(ParsedPairsArrayStart)
    {
        for(struct json_element *Element = ParsedPairsArrayStart->FirstSubElement;
            Element && (PairCount < MaxPairCount);
            Element = Element->NextSibling)
        {
            struct haversine_pair *Pair = (struct haversine_pair *)HaversinePairsStart.Data + PairCount++;

            struct json_element *X0 = LookUpJsonElement(Element, CONSTANT_STRING("x0"));
            struct json_element *Y0 = LookUpJsonElement(Element, CONSTANT_STRING("y0"));
            struct json_element *X1 = LookUpJsonElement(Element, CONSTANT_STRING("x1"));
            struct json_element *Y1 = LookUpJsonElement(Element, CONSTANT_STRING("y1"));
        }
    }


   

#if 0
    ValidateJsonAndSetCursor exits if invalid. Otherwise, it sets cursor to the first pair.
    ValidateJsonAndSetCursor(&Json);


    f64 Accumulator = 0;
    u64 PairCount = 0;

    // STOP: this seems to be reading the doubles properly. Next:
    //      - test with the small 10 pair dataset to confirm it is working properly
    //      - fix the project structure so that generator and parser can use the same functions
    //      - pull the code for reading the X and Y values into a function
    //          - it should return a bool to indicate if there are more pairs remaining
    //      - for getting the average let's keep a running tally of the number of pairs we encounter
    //          and divide by that number at the end. let's assume we don't know how many pairs there
    //          are in the file.
    //      - for fun test computing the sum as we go vs. storing them in an array and doing it all at once
    //              at the end to see which is faster.



    while(1)
    {
        // Parse returns true if there are more pairs to parse
        if(!Parse(&Json, &PairCount, &Accumulator))
        {
            break;
        }
    }

    f64 Result = Accumulator / (f64)PairCount;
    char PrintBuffer[256];
    sprintf(PrintBuffer, "Average: %lf", Result);
    OutputDebugStringA(PrintBuffer);
#endif

    if(Json.FilePointer) fclose(Json.FilePointer);
    if(Json.JsonToParse.Data) free((void *)Json.JsonToParse.Data);

    // A json input file containing 10 million pairs is 1,085,542,039 bytes, or a little more than 1 GB.
    // Therefore let's say that we support input json file sizes of no more than 2GB.

    return(0);
}

