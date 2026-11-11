#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <windows.h>

typedef double f64;
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int64_t s64;
typedef int32_t b32;

#define KILOBYTES(N) (size_t)(N * 1024)
#define MEGABYTES(N) (size_t)(KILOBYTES(N) * 1024)
#define GIGABYTES(N) (size_t)(MEGABYTES(N) * 1024)

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))
#define MAX_STRING_LEN 512

struct buffer
{
    size_t Count;
    u8 *Data;
};

static void
DebugOutput(const char *Format, ...)
{
    char PrintBuffer[512];
    int PrintBufferSize = sizeof(PrintBuffer);

    va_list Args;
    va_start(Args, Format);
    int BytesWritten = vsnprintf(PrintBuffer, PrintBufferSize, Format, Args);
    va_end(Args);

    if(BytesWritten > PrintBufferSize)
    {
        OutputDebugStringA("\n\nERROR: DebugOutput function format string did not fit in print buffer\n\n");
    }
    OutputDebugStringA(PrintBuffer);
    OutputDebugStringA("\n");

    return;
}

static buffer
AllocateBuffer(size_t NumBytes)
{
    buffer Result;
    Result.Data = (u8 *)malloc(NumBytes);
    if(Result.Data)
    {
        Result.Count = NumBytes;
        return(Result);
    }
    else
    {
        fprintf(stderr, "Malloc failed");
        exit(1);
    }
}

static void 
FreeBuffer(buffer *pBuffer)
{
    if(pBuffer->Data)
    {
        free(pBuffer->Data);
    }
    *pBuffer = {};
}
#include "platform_metrics.c"
#include "repetition_tester.cpp"
#include "read_overhead_test.cpp"
