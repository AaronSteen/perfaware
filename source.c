#include "common.h"

char *EffectiveAddressLUT[] = { "[bx + si", "[bx + di",  "[bp + si", "[bp + di", "[si", "[di", "[bp", "[bx" }; 
char *ByteRegLUT[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
char *WordRegLUT[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
char *SegRegLUT[] = {"es", "cs", "ss", "ds"};
extern char *MnemonicLUT[];
extern char *ByteRegLUT[];
extern char *WordRegLUT[];
extern char *SegRegLUT[];
extern char *EffectiveAddressLUT[];
extern u8 ByteOneToOpcodeEnumLUT[];
extern char *OpcodeEnumToStringLUT[];
extern u8 OpcodeLUT[];

void
__Debug_OutputErrorMessage(char *ErrorMessage, const char *CallingFunction, int Line)
{
    char ErrorBuffer[MAX_STRING_LEN];
    sprintf_s(ErrorBuffer, sizeof(ErrorBuffer), 
	    "\nERROR:\n  In function %s, on line %d,\n\n    ", CallingFunction, Line);
    OutputDebugStringA(ErrorBuffer);
    sprintf_s(ErrorBuffer, sizeof(ErrorBuffer), ErrorMessage);
    OutputDebugStringA(ErrorBuffer);
    OutputDebugStringA(".\n\n");
}


HANDLE
Win32_OpenFile(LPCSTR FilePath)
{
    HANDLE FileHandle = CreateFileA(FilePath, GENERIC_READ, 0,
            0, 3, FILE_ATTRIBUTE_READONLY, 0);
    if(FileHandle == INVALID_HANDLE_VALUE)
    {
        Debug_OutputErrorMessage("Error: Failed to open input file");
        exit(1);
    }

    return(FileHandle);
}

