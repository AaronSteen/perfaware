#ifndef FUNCTIONS_C

#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

#define NULLPTR (void *)0
#define NULLCHAR '\0'
#define MAX_STRING_LEN 255
#define REGISTER_NAME_LEN 2

typedef uint8_t u8;
typedef uint16_t u16;
typedef int8_t s8;
typedef int16_t s16;

#include "dispatch.c"
#include "sim.h"

SIZE_T
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


void
SteenCopy(struct string *Dest, char *Source, SIZE_T NumBytes)
{
    for(int i = 0; i < NumBytes; i++)
    {
	*Dest->Ptr++ = *Source++;
	Dest->Len++;
    }
}

void
WriteToOutBuf(struct outbuf *OutBuf, char *Source, SIZE_T NumBytes)
{
    if((u8 *)(OutBuf->Ptr + NumBytes) >= OutBuf->DoNotCrossThisLine)
    {
	Debug_OutputErrorMessage("OutBuf overflow", __func__, __LINE__);
	exit(1);
    }

    for(int i = 0; i < NumBytes; i++)
    {
	*OutBuf->Ptr++ = *Source++;
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
			Debug_OutputErrorMessage("Error: Failed to read decoded_instruction stream"
						    "into buffer", 
						    __func__, __LINE__);
			exit(1);
		    }
		}
		else
		{
		    Debug_OutputErrorMessage("Error: Failed to allocate memory"
					     "for decoded_instruction stream",
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
    bool UltimateWriteSuccessPlease = WriteFile(OutFileHandle, (LPCVOID)OutBuffer,
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

u8
ReadExtendedOpcode(struct decoded_inst *DecodedInst)
{
    u8 FirstByte = DecodedInst->Binary[0];
    u8 MeetTheDistinguisher = 0x38; //0011 1000
    u8 IHaveMetTheDistinguisher = DecodedInst->Binary[1] & MeetTheDistinguisher;

    switch(FirstByte)
    {
	case 0x80:
	case 0x81:
	{
	    switch(IHaveMetTheDistinguisher)
	    {
		case 0x00: // 000
		{
		    return(ADD);
		} break;

		case 0x08: // 001
		{
		    return(OR);
		} break;

		case 0x10: // 010
		{
		    return(ADC);
		} break;

		case 0x18: // 011
		{
		    return(SBB);
		} break;

		case 0x20: // 100
		{
		    return(AND);
		} break;

		case 0x28: // 101
		{
		    return(SUB);
		} break;

		case 0x30: // 110
		{
		    return(XOR);
		} break;

		case 0x38: // 111
		{
		    return(CMP);
		} break;

		default:
		{
		    exit(1);
		} break;
	    }
	} break;

	case 0x82:
	{
	    switch(IHaveMetTheDistinguisher)
	    {
		case 0x00: // 000
		{
		    return(ADD);
		} break;

		case 0x10: // 010
		{
		    return(ADC);
		} break;

		case 0x18: // 011
		{
		    return(SBB);
		} break;

		case 0x28: // 101
		{
		    return(SUB);
		} break;

		case 0x38: // 111
		{
		    return(CMP);
		} break;

		default:
		{
		    exit(1);
		} break;
	    }
	} break;

	case 0x83:
	{
	    switch(IHaveMetTheDistinguisher)
	    {
		case 0x00: // 000
		{
		    return(ADD);
		} break;

		case 0x10: // 010
		{
		    return(ADC);
		} break;

		case 0x18: // 011
		{
		    return(SBB);
		} break;

		case 0x28: // 101
		{
		    return(SUB);
		} break;

		case 0x38: // 111
		{
		    return(CMP);
		} break;

		default:
		{
		    exit(1);
		} break;
	    }
	} break;

	case 0xD0:
	case 0xD1:
	case 0xD2:
	case 0xD3:
	{
	    switch(IHaveMetTheDistinguisher)
	    {
		case 0x00: // 000
		{
		    return(ROL);
		} break;

		case 0x08: // 001
		{
		    return(ROR);
		} break;

		case 0x10: // 010
		{
		    return(RCL);
		} break;

		case 0x18: // 011
		{
		    return(RCR);
		} break;

		case 0x20: // 100
		{
		    return(SALorSHL);
		} break;

		case 0x28: // 101
		{
		    return(SHR);
		} break;

		case 0x38: // 111
		{
		    return(SAR);
		} break;

		default:
		{
		    exit(1);
		} break;
	    }
	} break;

	case 0xF6:
	case 0xF7:
	{
	    switch(IHaveMetTheDistinguisher)
	    {
		case 0x00: // 000
		{
		    return(TEST);
		} break;

		case 0x10: // 010
		{
		    return(NOT);
		} break;

		case 0x18: // 011
		{
		    return(NEG);
		} break;

		case 0x20: // 100
		{
		    return(MUL);
		} break;

		case 0x28: // 101
		{
		    return(IMUL);
		} break;

		case 0x30: // 110
		{
		    return(DIV);
		} break;

		case 0x38: // 111
		{
		    return(IDIV);
		} break;

		default:
		{
		    exit(1);
		} break;
	    }
	} break;

	case 0xFE:
	{
	    switch(IHaveMetTheDistinguisher)
	    {
		case 0x00: // 000
		{
		    return(INC);
		} break;

		case 0x10: // 010
		{
		    return(DEC);
		} break;

		default:
		{
		    exit(1);
		} break;
	    }
	} break;

	case 0xFF:
	{
	    switch(IHaveMetTheDistinguisher)
	    {
		case 0x00: // 000
		{
		    return(INC);
		} break;

		case 0x08: // 001
		{
		    return(DEC);
		} break;

		case 0x10: // 010
		case 0x18: // 011
		{
		    return(CALL);
		} break;

		case 0x20: // 100
		case 0x28: // 101
		{
		    return(JMP);
		} break;

		case 0x30: // 110
		{
		    return(PUSH);
		} break;

		default:
		{
		    exit(1);
		} break;
	    }
	} break;

	default:
	{
	    exit(1);
	} break;
    }
}

void
DecodeMOV(struct decoded_inst *DecodedInst)
{
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];
    struct parsed_mov ParsedMov = {0};
    ParsedMov.Binary = DecodedInst->Binary;
    ParsedMov.DestFlag = (bool)((ByteOne & 0x02) >> 1);
    ParsedMov.IsWord = (bool)(ByteOne & 0x01);
    ParsedMov.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedMov.Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedMov.RorM = (ByteTwo & R_OR_M_FIELD);

    struct string DecodedReg = {0};
    DecodedReg.Ptr = DecodedReg.String;
    struct string DecodedRorM = {0};
    DecodedRorM.Ptr = DecodedRorM.String;

    if( (ByteOne >= MEMorREG_TOorFROM_REG) && (ByteOne < SEGREG_TO_REGorMEM) )
    {
	DecodedInst->Size = 2;
	char *WhichRegIsIt = DecodeReg(ParsedMov.IsWord, ParsedMov.Reg);
	SteenCopy(&DecodedReg, WhichRegIsIt, REGISTER_NAME_LEN);

	DecodeRorM(&ParsedMov, &DecodedRorM);
	if(ParsedMov.Mod == MEM_MODE_DISP8)
	{
	    DecodedInst->Size = 3;
	}
	else if( (ParsedMov.Mod == MEM_MODE_DISP16) ||
		    (ParsedMov.Mod == MEM_MODE_NO_DISP && ParsedMov.RorM == DIRECT_ADDRESS) )
	{
	    DecodedInst->Size = 4;
	}
	if(ParsedMov.DestFlag)
	{
	    SteenCopy(&DecodedInst->OperandOne, DecodedReg.String, DecodedReg.Len);
	    SteenCopy(&DecodedInst->OperandTwo, DecodedRorM.String, DecodedRorM.Len);
	}
	else
	{
	    SteenCopy(&DecodedInst->OperandOne, DecodedRorM.String, DecodedRorM.Len);
	    SteenCopy(&DecodedInst->OperandTwo, DecodedReg.String, DecodedReg.Len);
	}
    }

    else if( (ByteOne >= SEGREG_TO_REGorMEM) && (ByteOne < REGorMEM_TO_SEGREG) )
    {
	Debug_OutputErrorMessage("SEGREG_TO_REGorMEM", __func__, __LINE__);
	// Not yet
    }
    
    else if( (ByteOne >= REGorMEM_TO_SEGREG) && (ByteOne < MEM_TO_ACCUM) )
    {
	Debug_OutputErrorMessage("REGorMEM_TO_SEGREG", __func__, __LINE__);
	// Not yet
    }

    else if( (ByteOne >= MEM_TO_ACCUM) && (ByteOne < ACCUM_TO_MEM) )
    {
	DecodedInst->Size = 3;

	if(ParsedMov.IsWord)
	{
	    SteenCopy(&DecodedReg, "ax", 2);
	}
	else
	{
	    SteenCopy(&DecodedReg, "al", 2);
	}

	SteenCopy(&DecodedRorM, "[", 1);
	char DirectAddressBuffer[MAX_STRING_LEN];
	s16 DirectAddress = *(s16 *)(ParsedMov.Binary + 1);
	int NumDirectAddrChars = Get16BitsAsChars(DirectAddressBuffer, DirectAddress);
	SteenCopy(&DecodedRorM, DirectAddressBuffer, NumDirectAddrChars);
	SteenCopy(&DecodedRorM, "]", 1);

	SteenCopy(&DecodedInst->OperandOne, DecodedReg.String, DecodedReg.Len);
	SteenCopy(&DecodedInst->OperandTwo, DecodedRorM.String, DecodedRorM.Len);
    }

    else if( (ByteOne >= ACCUM_TO_MEM) && (ByteOne < IMM_TO_REG) )
    {
	DecodedInst->Size = 3;

	if(ParsedMov.IsWord)
	{
	    SteenCopy(&DecodedReg, "ax", 2);
	}
	else
	{
	    SteenCopy(&DecodedReg, "al", 2);
	}

	SteenCopy(&DecodedRorM, "[", 1);
	char DirectAddressBuffer[MAX_STRING_LEN];
	s16 DirectAddress = *(s16 *)(ParsedMov.Binary + 1);
	int NumDirectAddrChars = Get16BitsAsChars(DirectAddressBuffer, DirectAddress);
	SteenCopy(&DecodedRorM, DirectAddressBuffer, NumDirectAddrChars);
	SteenCopy(&DecodedRorM, "]", 1);

	SteenCopy(&DecodedInst->OperandOne, DecodedRorM.String, DecodedRorM.Len);	
	SteenCopy(&DecodedInst->OperandTwo, DecodedReg.String, DecodedReg.Len);
    }

    else if( (ByteOne >= IMM_TO_REG) && (ByteOne < IMM_TO_REGorMEM) )
    {
	ParsedMov.IsWord = (bool)((ByteOne & 0x08) >> 3);
	ParsedMov.Reg = (ByteOne & 0x07);

	char *WhichRegIsIt = DecodeReg(ParsedMov.IsWord, ParsedMov.Reg);
	SteenCopy(&DecodedReg, WhichRegIsIt, REGISTER_NAME_LEN);

	char ImmediateValueBuffer[MAX_STRING_LEN];
	int NumValueChars = 0;

	if(ParsedMov.IsWord)
	{
	    s16 ImmediateValue = *(s16 *)(ParsedMov.Binary + 1);
	    NumValueChars = Get16BitsAsChars(ImmediateValueBuffer, ImmediateValue);
	    DecodedInst->Size = 3;
	}
	else
	{
	    s8 ImmediateValue = *(s8 *)(ParsedMov.Binary + 1);
	    NumValueChars = Get8BitsAsChars(ImmediateValueBuffer, ImmediateValue);
	    DecodedInst->Size = 2;
	}

	SteenCopy(&DecodedInst->OperandOne, DecodedReg.String, DecodedReg.Len);
	SteenCopy(&DecodedInst->OperandTwo, ImmediateValueBuffer, NumValueChars);
    }
    else if( (ByteOne >= IMM_TO_REGorMEM) && (ByteOne <= (IMM_TO_REGorMEM + 1)) )
    {
	DecodeRorM(&ParsedMov, &DecodedRorM);
	int NumDispBytes = 0;
	DecodedInst->Size = 3;

	if(ParsedMov.Mod == MEM_MODE_DISP8)
	{
	    DecodedInst->Size += 1;
	    NumDispBytes = 1;
	}
	else if(ParsedMov.Mod == MEM_MODE_DISP16)
	{
	    DecodedInst->Size += 2;
	    NumDispBytes = 2;
	}

	struct string ExplicitSize = {0};
	ExplicitSize.Ptr = ExplicitSize.String;
	char ImmediateValueBuffer[MAX_STRING_LEN];
	int NumImmediateChars = 0;

	if(ParsedMov.IsWord)
	{
	    DecodedInst->Size += 1;
	    SteenCopy(&ExplicitSize, "word ", 5);
	    s16 ImmediateValue = *(s16 *)(ParsedMov.Binary + NumDispBytes + 2);
	    NumImmediateChars = Get16BitsAsChars(ImmediateValueBuffer, ImmediateValue);
	    SteenCopy(&ExplicitSize, ImmediateValueBuffer, NumImmediateChars);
	}
	else
	{	    
	    SteenCopy(&ExplicitSize, "byte ", 5);
	    s8 ImmediateValue = *(s8 *)(ParsedMov.Binary + NumDispBytes + 2);
	    NumImmediateChars = Get8BitsAsChars(ImmediateValueBuffer, ImmediateValue);
	    SteenCopy(&ExplicitSize, ImmediateValueBuffer, NumImmediateChars);
	}

	SteenCopy(&DecodedInst->OperandOne, DecodedRorM.String, DecodedRorM.Len);
	SteenCopy(&DecodedInst->OperandTwo, ExplicitSize.String, ExplicitSize.Len);
    }
    else
    {
	// Error
	exit(1);
    }

}

char * 
GetEffectiveAddress(u8 RorMField)
{
    return(EffectiveAddressLUT[RorMField]);
}

int
Get8BitsAsChars(char *PrintBuffer, s8 Bits)
{
    int BytesWritten = sprintf_s(PrintBuffer, sizeof(PrintBuffer), "%d", Bits);
    return(BytesWritten);
}

int
Get16BitsAsChars(char *PrintBuffer, s16 Bits)
{
    int BytesWritten = sprintf_s(PrintBuffer, sizeof(PrintBuffer), "%d", Bits);
    return(BytesWritten);
}

char *
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

void
DecodeRorM(struct parsed_mov *ParsedMov, struct string *DecodedRorM)
{
    if(ParsedMov->Mod == REG_MODE)
    {
	char *WhichRegIsIt = DecodeReg(ParsedMov->IsWord, ParsedMov->RorM);
	SteenCopy(DecodedRorM, WhichRegIsIt, REGISTER_NAME_LEN);
	return;
    }
    else
    {
	if( (ParsedMov->Mod == MEM_MODE_NO_DISP) && (ParsedMov->RorM == DIRECT_ADDRESS) )
	{
	    // special case
	    SteenCopy(DecodedRorM, "[", 1);
	    char DirectAddressBuffer[MAX_STRING_LEN];
	    s16 DirectAddress = *(s16 *)(ParsedMov->Binary + 2);
	    int NumDirectAddrChars = Get16BitsAsChars(DirectAddressBuffer, DirectAddress);
	    SteenCopy(DecodedRorM, DirectAddressBuffer, NumDirectAddrChars);
	    SteenCopy(DecodedRorM, "]", 1);
	    return;
	}
	char *DestAddr = GetEffectiveAddress(ParsedMov->RorM);
	SIZE_T DestAddrLen = GetZTermStringLen(DestAddr);
	SteenCopy(DecodedRorM, DestAddr, DestAddrLen);

	if(ParsedMov->Mod == MEM_MODE_NO_DISP)
	{
	    // If no displacement just close the bracket
	    SteenCopy(DecodedRorM, "]", 1);
	    return;
	}
	else
	{
	    char DispValueBuffer[MAX_STRING_LEN];
	    int NumDispChars = 0;
	    bool IsNegative = false;

	    if(ParsedMov->Mod == MEM_MODE_DISP8)
	    {
		// something wrong 
		s8 DispValue = *(s8 *)(ParsedMov->Binary + 2);
		if(DispValue == 0)
		{
		    SteenCopy(DecodedRorM, "]", 1);
		    return;
		}
		else if(DispValue < 0)
		{
		    IsNegative = true;
		}
		NumDispChars = Get8BitsAsChars(DispValueBuffer, DispValue);
	    }
	    else if(ParsedMov->Mod == MEM_MODE_DISP16)
	    {
		s16 DispValue = *(s16 *)(ParsedMov->Binary + 2);
		if(DispValue == 0)
		{
		    SteenCopy(DecodedRorM, "]", 1);
		    return;
		}
		else if(DispValue < 0)
		{
		    IsNegative = true;
		}
		NumDispChars = Get16BitsAsChars(DispValueBuffer, DispValue);
	    }
	    else
	    {
		// Error
		exit(1);
	    }

	    if(IsNegative)
	    {
		SteenCopy(DecodedRorM, " - ", 3);
		SteenCopy(DecodedRorM, ((char *)DispValueBuffer + 1), (NumDispChars - 1));
	    }
	    else
	    {
		SteenCopy(DecodedRorM, " + ", 3);
		SteenCopy(DecodedRorM, (char *)DispValueBuffer, NumDispChars);
	    }

	    SteenCopy(DecodedRorM, "]", 1);
	}
    }

}

#define FUNCTIONS_C
#endif
