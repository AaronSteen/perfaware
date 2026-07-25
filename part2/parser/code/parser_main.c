#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <windows.h>

typedef double f64;
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

#define KILOBYTES(N) (size_t)(N * 1024)
#define MEGABYTES(N) (size_t)(KILOBYTES(N) * 1024)
#define GIGABYTES(N) (size_t)(MEGABYTES(N) * 1024)

struct json
{
    FILE *FilePointer;
    char *Filename;
    size_t Size;
    u8 *Start;
    u8 *Cursor;
};

#define OutputErrorMessage(...) \
    OutputErrorMessage_(__func__, __LINE__, __VA_ARGS__)

void
OutputErrorMessage_(const char *CallingFunction, int Line, const char *Format, ...)
{
    char Buffer[512];
    int Size = sizeof(Buffer);
    
    int Offset = snprintf(Buffer, Size,
                          "\nERROR:\n    In function %s, on line %d,\n\n    ",
                          CallingFunction, Line);
    if(Offset < 0)
    {
        // snprint returns negative if it failed to print anything
        Offset = 0;
    }
    else if(Offset > Size-1)
    {
        Offset = Size-1;
    }

    va_list Args;
    va_start(Args, Format);
    int Written = vsnprintf(Buffer + Offset, Size - Offset, Format, Args);
    va_end(Args);

    if(Written > 0)
    {
        Offset += Written;
    }
    if(Offset > Size-1)
    {
        Offset = Size-1;
    }

    snprintf(Buffer+Offset, Size-Offset, ".\n\n");

    OutputDebugStringA(Buffer);
}

char *
GetFileExtension(char *Filename)
{
    char *Extension = Filename;
    while(*Extension != '\0')
    {
        ++Extension;
    }
    while(*Extension != '.')
    {
        --Extension;
    }
    return(Extension);
}

static void
OpenJson(struct json *Json)
{
    Json->FilePointer = fopen(Json->Filename, "rb");
    if(!Json->FilePointer)
    {
        OutputErrorMessage("Failed to open file %s", Json->Filename);
        exit(1);
    }

    int SeekResult = _fseeki64(Json->FilePointer, 0, SEEK_END);
    if(SeekResult != 0)
    {
        OutputErrorMessage("Failed to seek to end of file %s", Json->Filename);
        exit(1);
    }

    Json->Size = _ftelli64(Json->FilePointer);
    if(SeekResult == -1L) 
    {
        // -1LU is result on fail
        OutputErrorMessage("Couldn't get size for file %s", Json->Filename);
        exit(1);
    }
}

int
main(int ArgCount, char **ArgVector)
{
    if(ArgCount != 2)
    {
        OutputErrorMessage("Two arguments required, but %d was/were supplied", ArgCount);
        exit(1);
    }

    struct json Json;
    Json.Filename = ArgVector[1];
    char *Extension = GetFileExtension(Json.Filename);
    if(strcmp(Extension, ".json") != 0)
    {
        OutputErrorMessage("This utility only parses .json files. You supplied a file with extension %s", Extension);
        exit(1);
    }

    OpenJson(&Json);
    u64 TwoGigs = GIGABYTES(2);
    if(Json.Size > TwoGigs)
    {
        OutputErrorMessage("This utility only parses .json files of size %llu or smaller. You supplied a file of size %llu", GIGABYTES(2), Json.Size);
        exit(1);
    }

    // A json input file containing 10 million pairs is 1,085,542,039 bytes, or a little more than 1 GB.
    // Therefore let's say that we support input json file sizes of no more than 2GB.



    return(0);
}

