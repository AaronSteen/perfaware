#include "sim.h"

internal void
Debug_PrintBinary(u8 Value)
{
    for(int i = 0; i < 8; i++)
    {
        int ShiftAmt = (7 - i);
        u8 Digit = ((Value >> ShiftAmt) & 1);
        if(Digit)
        {
            OutputDebugStringA("1");
        }
        else
        {
            OutputDebugStringA("0");
        }
        if(i == 3)
        {
            OutputDebugStringA(" ");
        }
    }
}

internal void
__Debug_OutputErrorMessage(char *ErrorMessage, char *CallingFunction, int Line)
{
    char ErrorBuffer[MAX_STRING_LEN];
    sprintf_s(ErrorBuffer, sizeof(ErrorBuffer), 
            "\nERROR:\n  In function %s, on line %d,\n\n    ", CallingFunction, Line);
    OutputDebugStringA(ErrorBuffer);
    sprintf_s(ErrorBuffer, sizeof(ErrorBuffer), ErrorMessage);
    OutputDebugStringA(ErrorBuffer);
    OutputDebugStringA(".\n\n");
}

internal void
Debug_PrintCurrentStatus(struct decoded_inst *DecodedInst)
{
    /*char ErrorBuffer[MAX_STRING_LEN];*/
    /**/
    /*OutputDebugStringA("\n\n");*/
    /**/
    /*OutputDebugStringA("Just decoded inst, Hex: ");*/
    /*sprintf_s(ErrorBuffer, sizeof(ErrorBuffer), "0x%02X", DecodedInst->Binary[0]);*/
    /*OutputDebugStringA(ErrorBuffer);*/
    /*OutputDebugStringA("\n\n");*/

    for(int i = 0; i < 10; i++)
    {
        char NumByteToPrintBuf[MAX_STRING_LEN];
        int NumByteToPrint;
        if(i < DecodedInst->Size)
        {
            NumByteToPrint = i + 1;
        }
        else
        {
            NumByteToPrint = ((i - DecodedInst->Size) + 1);
        }

        sprintf_s(NumByteToPrintBuf, sizeof(NumByteToPrintBuf), "Inst byte %d:", NumByteToPrint);
        OutputDebugStringA(NumByteToPrintBuf);
        u8 InstByte = *((DecodedInst->Binary) + i);
        Debug_PrintBinary(InstByte);
        OutputDebugStringA("\n\n");
        if(i == (DecodedInst->Size - 1))
        {
            OutputDebugStringA("\n============================\n");

            u8 NextInstByteOne = *(DecodedInst->Binary + DecodedInst->Size);
            u8 NextInstOpcodeEnum = ByteOneToOpcodeEnumLUT[NextInstByteOne];
            char *NextInstOpcodeString;
            if(NextInstOpcodeEnum == EXTENDED)
            {
                NextInstOpcodeString = "Extended";
            }
            else
            {
                NextInstOpcodeString = OpcodeEnumToStringLUT[NextInstOpcodeEnum];
            }
            OutputDebugStringA("\n    Next inst: ");
            OutputDebugStringA(NextInstOpcodeString);
        }
        OutputDebugStringA("\n\n");
    }
    OutputDebugStringA("\n\n");

}

internal void
InitString(struct string *String)
{
    String->Len = 0;
    String->Ptr = String->Start;
    String->DoNotCrossThisLine = String->Start + MAX_STRING_LEN;
}

internal char *
DecodeReg(bool WordFlag, u8 RegCode)
{
    if(WordFlag)
    {
        return(WordRegLUT[RegCode]);
    }
    else
    {
        return(ByteRegLUT[RegCode]);    
    }
}

internal void
GetIntAsChars(struct string *IntToPrintString, struct int_to_print IntToPrint)
{
    switch(IntToPrint->Type)
    {
        case(SIGNED_8):
        {
            IntToPrintString.Len = sprintf_s(IntToPrintString.Ptr, sizeof(PrintBuffer), 
                    "%d", IntToPrint->Sint8);
        } break;

        case(UNSIGNED_8):
        {
            IntToPrintString.Len = sprintf_s(IntToPrintString.Ptr, sizeof(PrintBuffer), 
                    "%d", IntToPrint->Uint8);
        } break;

        case(SIGNED_16):
        {
            IntToPrintString.Len = sprintf_s(IntToPrintString.Ptr, sizeof(PrintBuffer), 
                    "%d", IntToPrint->Sint16);
        } break;

        case(UNSIGNED_16):
        {
            IntToPrintString.Len = sprintf_s(IntToPrintString.Ptr, sizeof(PrintBuffer), 
                    "%d", IntToPrint->Uint16);
        } break;
    }
}

internal SIZE_T
GetZTermStringLen(char *String)
{
    char *p = String;
    SIZE_T Len = 0;

    while(*p != NULLCHAR)
    {
        Len++;
        p++;
    }

    return(Len);
}


    internal HANDLE 
Win32_OpenFile(LPCSTR FilePath)
{
    HANDLE FileHandle = CreateFileA(FilePath, GENERIC_READ, 0,
            0, 3, FILE_ATTRIBUTE_READONLY, 0);
    if(FileHandle == INVALID_HANDLE_VALUE)
    {
        Debug_OutputErrorMessage("Error: Failed to open open input file");
        exit(1);
    }

    return(FileHandle);
}

    internal void
Win32_LoadInstStream(HANDLE FileHandle, struct istream *IStream)
{
    if(FileHandle)
    {
        GetFileSizeEx(FileHandle, &IStream->StreamSize);
        if(IStream->StreamSize.QuadPart)
        {
            HANDLE HeapHandle = GetProcessHeap();
            if(HeapHandle)
            {
                IStream->Start = (u8 *)HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, 
                        (SIZE_T)IStream->StreamSize.LowPart);
                if(IStream->Start)
                {
                    DWORD BytesRead = 0;
                    bool Result = ReadFile(FileHandle, IStream->Start, 
                            IStream->StreamSize.LowPart, &BytesRead, 0);
                    if((BytesRead == IStream->StreamSize.LowPart) && Result)
                    {
                        IStream->DoNotCrossThisLine = ((u8 *)IStream->Start +
                                IStream->StreamSize.LowPart);
                    }
                    else
                    {
                        __Debug_OutputErrorMessage("Error: Failed to read decoded_instruction stream"
                                "into buffer", 
                                __func__, __LINE__);
                        exit(1);
                    }
                }
                else
                {
                    __Debug_OutputErrorMessage("Error: Failed to allocate memory"
                            "for decoded_instruction stream",
                            __func__, __LINE__);
                    exit(1);
                }
            }
            else
            {
                __Debug_OutputErrorMessage("Error: Failed to get process heap handle", 
                        __func__, __LINE__);
                exit(1);
            }
        }
        else
        {
            __Debug_OutputErrorMessage("Error: Failed to read input file size", 
                    __func__, __LINE__);
            exit(1);
        }
    }
    else
    {
        __Debug_OutputErrorMessage("Error: Bad input file handle", 
                __func__, __LINE__);
        exit(1);
    }

}

    internal void
Win32_WriteFile(u8 *OutBuffer, SIZE_T OutBufferSize)
{
    char *OutFilePath = "out.asm";
    HANDLE OutFileHandle = CreateFileA((LPCSTR)OutFilePath, GENERIC_WRITE,
            0, 0, CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL, 0);

    DWORD BytesWritten = 0;
    bool UltimateWriteSuccessPlease = WriteFile(OutFileHandle, (LPCVOID)OutBuffer,
            (DWORD)OutBufferSize, &BytesWritten, 0);

    if( !(UltimateWriteSuccessPlease && (BytesWritten == OutBufferSize)) )
    {
        Debug_OutputErrorMessage("Error writing outfile");
        exit(1);
    }
}


internal void
WriteToOutBuf(char *Source, SIZE_T NumBytes)
{
    if((u8 *)(OutBuf.Ptr + NumBytes) >= OutBuf.DoNotCrossThisLine)
    {
	Debug_OutputErrorMessage("OutBuf overflow");
	exit(1);
    }

    for(int i = 0; i < NumBytes; i++)
    {
	*OutBuf.Ptr++ = *Source++;
    }
}
