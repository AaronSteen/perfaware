#pragma once

#define _CRT_SECURE_NO_WARNINGS

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

#define EARTH_RADIUS 6372.8f

#define Assert(Condition) if(!(Condition)) {*(volatile int *)0 = 1;}

struct buffer
{
    size_t NumBytes;
    u8 *Data;
};

#define CONSTANT_STRING(String) (struct buffer){.NumBytes = (sizeof(String)-1), .Data = (u8 *)String}

struct json
{
    FILE *FilePointer;
    char *Filename;
    struct buffer JsonToParse;
    u64 Cursor;
};

struct haversine_pair
{
    f64 X0;
    f64 Y0;
    f64 X1;
    f64 Y1;
};

