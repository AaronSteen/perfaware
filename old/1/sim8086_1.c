#include <stdio.h>
#include <windows.h>
#include <stdint.h>

#define NULLPTR (void *)0
#define NULLCHAR '\0'
#define MAX_STRING_LEN 255
#define REGISTER_NAME_LEN 2

typedef uint8_t u8;
typedef uint16_t u16;

enum 
{
    OPCODE_FIELD = 0xFC,
    REG_IS_DEST_FLAG = 0x02,
    WORD_FLAG = 0x01,
    MOD_FIELD = 0xC0,
    REG_FIELD = 0x38,
    R_OR_M_FIELD = 0x07
} fields;

struct inst_stream
{
    LARGE_INTEGER StreamSize;
    u8 *DoNotCrossThisLine;
    u16 *Instructions;
};

struct inst
{
    u8 Opcode;
    u8 RegIsDest;
    u8 IsWord;
    u8 OpcodeMod;
    u8 Reg;
    u8 RorM;
};

char *ByteRegLUT[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
char *WordRegLUT[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};

u8 *Win32_ReadFile(HANDLE FileHandle);
HANDLE Win32_OpenFile(LPCSTR FilePath);
void Debug_OutputErrorMessage(char *ErrorMessage, char *CallingFunction, int Line);
void SteenCopy(char *Dest, char *Source, SIZE_T NumBytes);
SIZE_T SteenLen(char *String);
void Win32_WriteFile(u8 *OutBuffer, SIZE_T OutBufferSize);
void Win32_LoadInstStream(HANDLE FileHandle, struct inst_stream *InstStream);
char *DisassembleInst(struct inst *Inst, struct inst_stream *InstStream, char *OutBufPtr);

int 
main(int argc, char **argv)
{
    if(argc != 2)
    {
	Debug_OutputErrorMessage("Error: Usage", __func__, __LINE__);
	exit(1);
    }

    LPCSTR FilePath = argv[1];
    HANDLE FileHandle = Win32_OpenFile(FilePath);
    struct inst_stream InstStream = {};
    
    Win32_LoadInstStream(FileHandle, &InstStream);
    u16 *InstPtr = InstStream.Instructions;
    SIZE_T NumInstructions = 0;

    while((u8 *)InstPtr < InstStream.DoNotCrossThisLine)
    {
	if(*InstPtr)
	{
	    NumInstructions++;
	    InstPtr++;
	}
	else
	{
	    Debug_OutputErrorMessage("Error: Read empty instruction", __func__, __LINE__);
	    exit(1);
	}
    }

    InstPtr = InstStream.Instructions;

    char *Directive = "bits 16\n\n";
    HANDLE HeapHandle = GetProcessHeap();
    char *OutBuf = (char *)HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, 100000);
    char *OutBufPtr = OutBuf;
    char *OutBuf_DoNotCross = OutBuf + 100000;
    
    // write directive
    SIZE_T DirectiveLen = SteenLen(Directive);
    SteenCopy(OutBufPtr, Directive, DirectiveLen);
    OutBufPtr += DirectiveLen;

    for(int i = 0; i < NumInstructions; i++)
    {
	struct inst Inst;
	Inst.Opcode = (u8)*InstPtr & OPCODE_FIELD;
	Inst.RegIsDest = (u8)*InstPtr & REG_IS_DEST_FLAG;
	Inst.IsWord = (u8)*InstPtr & WORD_FLAG;
	Inst.OpcodeMod = (u8)((*InstPtr >> 8) & MOD_FIELD);
	Inst.Reg = (u8)((*InstPtr >> 8) & REG_FIELD);
	Inst.RorM = (u8)((*InstPtr >> 8) & R_OR_M_FIELD);

	if( (OutBufPtr + SteenLen("mov xx, xx\n")) >= OutBuf_DoNotCross )
	{
	    Debug_OutputErrorMessage("Error: OutBuf overflow", __func__, __LINE__);
	    exit(1);
	}
	// DisassembleInst passes back OutBufPtr
	OutBufPtr = DisassembleInst(&Inst, &InstStream, OutBufPtr);
	if(i != (NumInstructions - 1))
	{
	    *OutBufPtr = '\n';
	    OutBufPtr++;
	}
	InstPtr++;
    }
    SIZE_T OutFileSize = (OutBufPtr - OutBuf);

    Win32_WriteFile((u8 *)OutBuf, OutFileSize);

    return(0);
}

SIZE_T
SteenLen(char *String)
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


void
SteenCopy(char *Dest, char *Source, SIZE_T NumBytes)
{
    for(int i = 0; i < NumBytes; i++)
    {
	*Dest++ = *Source++;
    }
}

HANDLE 
Win32_OpenFile(LPCSTR FilePath)
{
    HANDLE FileHandle = CreateFileA(FilePath, GENERIC_READ, 0,
			     0, 3, FILE_ATTRIBUTE_READONLY, 0);
    if(FileHandle == INVALID_HANDLE_VALUE)
    {
	Debug_OutputErrorMessage("Error: Failed to open open input file", __func__, __LINE__);
	exit(1);
    }

    return(FileHandle);
}

void
Win32_LoadInstStream(HANDLE FileHandle, struct inst_stream *InstStream)
{
    if(FileHandle)
    {
	GetFileSizeEx(FileHandle, &InstStream->StreamSize);
	if(InstStream->StreamSize.QuadPart)
	{
	    HANDLE HeapHandle = GetProcessHeap();
	    if(HeapHandle)
	    {
		InstStream->Instructions = (u16 *)HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, 
					     (SIZE_T)InstStream->StreamSize.LowPart);
		if(InstStream->Instructions)
		{
		    DWORD BytesRead = 0;
		    BOOL Result = ReadFile(FileHandle, InstStream->Instructions, 
					    InstStream->StreamSize.LowPart, &BytesRead, 0);
		    if((BytesRead == InstStream->StreamSize.LowPart) && Result)
		    {
			InstStream->DoNotCrossThisLine = ((u8 *)InstStream->Instructions +
						    InstStream->StreamSize.LowPart);
		    }
		    else
		    {
			Debug_OutputErrorMessage("Error: Failed to read instruction stream"
						    "into buffer", 
						    __func__, __LINE__);
			exit(1);
		    }
		}
		else
		{
		    Debug_OutputErrorMessage("Error: Failed to allocate memory"
					     "for instruction stream",
						__func__, __LINE__);
		    exit(1);
		}
	    }
	    else
	    {
		Debug_OutputErrorMessage("Error: Failed to get process heap handle", 
					    __func__, __LINE__);
		exit(1);
	    }
	}
	else
	{
	    Debug_OutputErrorMessage("Error: Failed to read input file size", 
					__func__, __LINE__);
	    exit(1);
	}
    }
    else
    {
	Debug_OutputErrorMessage("Error: Bad input file handle", 
				    __func__, __LINE__);
	exit(1);
    }

}

void
Win32_WriteFile(u8 *OutBuffer, SIZE_T OutBufferSize)
{
    char *OutFilePath = "out.asm";
    HANDLE OutFileHandle = CreateFileA((LPCSTR)OutFilePath, GENERIC_WRITE,
					0, 0, CREATE_NEW,
					FILE_ATTRIBUTE_NORMAL, 0);

    DWORD BytesWritten = 0;
    BOOL UltimateWriteSuccessPlease = WriteFile(OutFileHandle, (LPCVOID)OutBuffer,
				    (DWORD)OutBufferSize, &BytesWritten, 0);

    if( !(UltimateWriteSuccessPlease && (BytesWritten == OutBufferSize)) )
    {
	Debug_OutputErrorMessage("Error writing outfile", __func__, __LINE__);
	exit(1);
    }
}

void
Debug_OutputErrorMessage(char *ErrorMessage, char *CallingFunction, int Line)
{
    char ErrorBuffer[MAX_STRING_LEN];
    sprintf_s(ErrorBuffer, sizeof(ErrorBuffer), 
	    "\nERROR:\n  In function %s, on line %d,\n\n    ", CallingFunction, Line);
    OutputDebugStringA(ErrorBuffer);
    sprintf_s(ErrorBuffer, sizeof(ErrorBuffer), ErrorMessage);
    OutputDebugStringA(ErrorBuffer);
    OutputDebugStringA(".\n\n");
}

char *
DisassembleInst(struct inst *Inst, struct inst_stream *InstStream, char *OutBufPtr)
{
    if( (Inst->Opcode != 0x88) || (Inst->OpcodeMod != 0xC0) || 
		(Inst->RegIsDest == REG_IS_DEST_FLAG) )
    {
	Debug_OutputErrorMessage("Error: Unrecognized opcode", __func__, __LINE__);
	exit(1);
    }

    char *Opcode = "mov ";
    SIZE_T OpcodeLen = SteenLen(Opcode);
    SteenCopy(OutBufPtr, Opcode, OpcodeLen);
    OutBufPtr += OpcodeLen;

    // write operands
    u8 DestRegCode = Inst->RorM;
    char *DestReg = ByteRegLUT[DestRegCode];
    u8 SourceRegCode = (Inst->Reg >> 3);
    char *SourceReg = ByteRegLUT[SourceRegCode];
    if(Inst->IsWord & WORD_FLAG)
    {
	DestReg = WordRegLUT[DestRegCode];
	SourceReg = WordRegLUT[SourceRegCode];
    }
    SteenCopy(OutBufPtr, DestReg, REGISTER_NAME_LEN);
    OutBufPtr += REGISTER_NAME_LEN;

    *OutBufPtr++ = ',';
    *OutBufPtr++ = ' ';

    SteenCopy(OutBufPtr, SourceReg, REGISTER_NAME_LEN);
    OutBufPtr += REGISTER_NAME_LEN;

    return(OutBufPtr);
}
