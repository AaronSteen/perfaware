#pragma once
#include "repetition.h"

#include <fcntl.h>
#include <io.h>

enum allocation_type
{
    AllocType_none,
    AllocType_malloc,

    AllocType_Count,
};

struct read_parameters
{
    allocation_type AllocType;
    buffer Dest;
    char const *pFilename;
};

typedef void read_overhead_test_func(repetition_tester *pTester, read_parameters *pParams);

static char const *
DescribeAllocationType(allocation_type AllocType)
{
    char const *Result;
    switch (AllocType) {
        case AllocType_none: {Result = "";} break;
        case AllocType_malloc: {Result = "malloc";} break;
        default : {Result = "UNKNOWN";} break;
    }

    return Result;
}

static void 
HandleAllocation(read_parameters *pParams, buffer *pBuffer)
{
    switch (pParams->AllocType) {
        case AllocType_none:
        {
        } break;
        
        case AllocType_malloc:
        {
            *pBuffer = AllocateBuffer(pParams->Dest.Count);
        } break;

        default:
        {
            fprintf(stderr, "ERROR: Unrecognized allocation type");
        } break;
    }
}

static void
HandleDeallocation(read_parameters *pParams, buffer *pBuffer)
{
    switch (pParams->AllocType) {
        case AllocType_none:
        {
        } break;
        
        case AllocType_malloc:
        {
            FreeBuffer(pBuffer);
        } break;

        default:
        {
            fprintf(stderr, "ERROR: Unrecognized allocation type");
        } break;
    }
}


static void
ReadViaFRead(repetition_tester *pTester, read_parameters *pParams)
{
    while (IsTesting(pTester)) {
        FILE *pFile = fopen(pParams->pFilename, "rb");
        if (pFile) {
            buffer DestBuffer = pParams->Dest;
            HandleAllocation(pParams, &DestBuffer);

            BeginTime(pTester);
            size_t Result = fread(DestBuffer.Data, DestBuffer.Count, 1, pFile);
            EndTime(pTester);

            if (Result == 1) {
                CountBytes(pTester, DestBuffer.Count);
            }
            else {
                Error(pTester, "fread failed");
            }
            HandleDeallocation(pParams, &DestBuffer);
            fclose(pFile);
        }
        else {
            Error(pTester, "fopen failed");
        }
    }
}

static void
ReadViaRead(repetition_tester *pTester, read_parameters *pParams)
{
    while (IsTesting(pTester)) {
        int File = _open(pParams->pFilename, _O_BINARY|_O_RDONLY);

        if (File != -1) {
            buffer DestBuffer = pParams->Dest;
            HandleAllocation(pParams, &DestBuffer);
            
            u8 *pDest = DestBuffer.Data;
            u64 SizeRemaining = DestBuffer.Count;
            while (SizeRemaining) {
                u32 ReadSize = INT_MAX;
                if ((u64)ReadSize > SizeRemaining) {
                    ReadSize = (u32)SizeRemaining;
                }

                BeginTime(pTester);
                int Result = _read(File, pDest, ReadSize);
                EndTime(pTester);

                if (Result == (int)ReadSize) {
                    CountBytes(pTester, ReadSize);
                }
                else {
                    Error(pTester, "_read failed");
                    break;
                }

                SizeRemaining -= ReadSize;
                pDest += ReadSize;
            }

            HandleDeallocation(pParams, &DestBuffer);
            _close(File);
        }
        else {
            Error(pTester, "_open failed");
        }
    }
}

static void
ReadViaReadFile(repetition_tester *pTester, read_parameters *pParams)
{
    while (IsTesting(pTester)) {
        HANDLE File = CreateFileA(pParams->pFilename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (File != INVALID_HANDLE_VALUE) {
            buffer DestBuffer = pParams->Dest;
            HandleAllocation(pParams, &DestBuffer);

            u64 SizeRemaining = pParams->Dest.Count;
            u8 *pDest = (u8 *)DestBuffer.Data;
            while (SizeRemaining) {
                u32 ReadSize = (u32)-1;
                if ((u64)ReadSize > SizeRemaining) {
                    ReadSize = (u32)SizeRemaining;
                }

                DWORD BytesRead = 0;
                BeginTime(pTester);
                BOOL Result = ReadFile(File, pDest, ReadSize, &BytesRead, 0);
                EndTime(pTester);

                if (Result && (BytesRead == ReadSize)) {
                    CountBytes(pTester, ReadSize);
                }
                else {
                    Error(pTester, "ReadFile failed");
                }

                SizeRemaining -= ReadSize;
                pDest += ReadSize;
            }

            HandleDeallocation(pParams, &DestBuffer);
            CloseHandle(File);
        }
        else {
            Error(pTester, "CreateFileA failed");
        }
    }
}
