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

void
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

void
Debug_PrintCurrentStatus(struct decoded_inst *DecodedInst, int IStreamIdx)
{
    // Print the instruction we just decoded
    char DecodedInstBuf[MAX_STRING_LEN] = {0};
    sprintf_s(DecodedInstBuf, sizeof(DecodedInstBuf), "\n         Idx %d         \n", IStreamIdx);
    OutputDebugStringA(DecodedInstBuf);
    sprintf_s(DecodedInstBuf, sizeof(DecodedInstBuf), "\n  *** %s %s%s %s ***  \n\n", 
                DecodedInst->Mnemonic, DecodedInst->OperandOne, 
                ( (DecodedInst->OperandTwo[0] != '\0') ? "," : "" ), DecodedInst->OperandTwo);
    OutputDebugStringA(DecodedInstBuf);

    // Print the bytes we just decoded, and a preview of the next few bytes of the instruction stream
    for(int i = 0; i < 10; i++)
    {
        char NumByteToPrintBuf[MAX_STRING_LEN] = {0};
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

struct istream
Win32_LoadIStream(HANDLE FileHandle)
{
    struct istream IStream = {0};

    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        GetFileSizeEx(FileHandle, &IStream.StreamSize);
        if(IStream.StreamSize.QuadPart)
        {
            HANDLE HeapHandle = GetProcessHeap();
            if(HeapHandle)
            {
                IStream.Start = (u8 *)HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, 
                        (SIZE_T)IStream.StreamSize.LowPart);
                if(IStream.Start)
                {
                    DWORD BytesRead = 0;
                    bool Result = ReadFile(FileHandle, IStream.Start, 
                            IStream.StreamSize.LowPart, &BytesRead, 0);
                    if((BytesRead == IStream.StreamSize.LowPart) && Result)
                    {
                        IStream.DoNotCrossThisLine = ((u8 *)IStream.Start +
                                IStream.StreamSize.LowPart);
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

    return IStream;
}

void
LookUpReg(struct parsed_inst *ParsedInst, char *StrBuffer)
{
    int DoNotOverflow = MAX_STRING_LEN - strlen(StrBuffer);
    if(ParsedInst->IsWord)
    {
        strncat(StrBuffer, WordRegLUT[ParsedInst->Reg], DoNotOverflow);
    }
    else
    {
        strncat(StrBuffer, ByteRegLUT[ParsedInst->Reg], DoNotOverflow);
    }
}

void
LookUpEffectiveAddress(struct parsed_inst *ParsedInst, char *StrBuffer)
{
    if(ParsedInst->IsWord)
    {
        strncat(StrBuffer, EffectiveAddressLUT[ParsedInst->RorM], MAX_STRING_LEN);
    }
    else
    {
        strncat(StrBuffer, EffectiveAddressLUT[ParsedInst->RorM], MAX_STRING_LEN);
    }
}

void
GetIntAsString_8(u8 UnsignedIntBits, char *StrBuffer, bool Signed)
{
    char Temp[MAX_STRING_LEN] = {0};
    if(Signed)
    {
        s8 SignedIntBits = (s8)UnsignedIntBits;
        sprintf_s(Temp, MAX_STRING_LEN, "%d", SignedIntBits);
    }
    else
    {
        sprintf_s(Temp, MAX_STRING_LEN, "%d", UnsignedIntBits);
    }
    size_t DoNotOverflow = (MAX_STRING_LEN - strlen(StrBuffer));
    strncat(StrBuffer, Temp, DoNotOverflow);
}


void
GetIntAsString_16(u16 UnsignedIntBits, char *StrBuffer, bool Signed)
{
    char Temp[MAX_STRING_LEN] = {0};
    if(Signed)
    {
        s16 SignedIntBits = (s16)UnsignedIntBits;
        sprintf_s(Temp, MAX_STRING_LEN, "%d", SignedIntBits);
    }
    else
    {
        sprintf_s(Temp, MAX_STRING_LEN, "%d", UnsignedIntBits);
    }
    size_t DoNotOverflow = (MAX_STRING_LEN - strlen(StrBuffer));
    strncat(StrBuffer, Temp, DoNotOverflow);
}

void
ReadRorMField(struct parsed_inst *ParsedInst, char *RorMBuffer)
{
    if(ParsedInst->Mod == REG_MODE)
    {
        int DoNotOverflow = MAX_STRING_LEN - strlen(RorMBuffer);
        if(ParsedInst->IsWord)
        {
            strncat(RorMBuffer, WordRegLUT[ParsedInst->RorM], DoNotOverflow);
        }
        else
        {
            strncat(RorMBuffer, ByteRegLUT[ParsedInst->RorM], DoNotOverflow);
        }
        return;
    }

    // Else it's a memory operation
    if(ParsedInst->Mod == MEM_MODE_NO_DISP)
    {
        if(ParsedInst->RorM == DIRECT_ADDRESS)
        {
            u16 DispBits = *(u16 *)(ParsedInst->Binary + 2);
            strncat(RorMBuffer, "[", MAX_STRING_LEN);
            GetIntAsString_16(DispBits, RorMBuffer, false);
        }
        else
        {
            LookUpEffectiveAddress(ParsedInst, RorMBuffer);
        }
    }
    else if(ParsedInst->Mod == MEM_MODE_DISP_8)
    {
        LookUpEffectiveAddress(ParsedInst, RorMBuffer);
        s8 DispBits = *(s8 *)(ParsedInst->Binary + 2);
        if(DispBits != 0)
        {
            char DispBuffer[MAX_STRING_LEN] = {0};

            bool IsNegative = (DispBits < 0);
            bool IsSigned = true;

            if(IsNegative)
            {
                strncat(RorMBuffer, " - ", 3);
                GetIntAsString_8(DispBits, DispBuffer, IsSigned);
                strncat( RorMBuffer, ((char *)(DispBuffer + 1)), (MAX_STRING_LEN - strlen(RorMBuffer)) );
            }
            else
            {
                strncat(RorMBuffer, " + ", 3);
                GetIntAsString_8(DispBits, DispBuffer, IsSigned);
                strncat( RorMBuffer, DispBuffer, (MAX_STRING_LEN - strlen(RorMBuffer)) );
            }
        }
    }
    else if(ParsedInst->Mod == MEM_MODE_DISP_16)
    {
        LookUpEffectiveAddress(ParsedInst, RorMBuffer);
        s16 DispBits = *(s16 *)(ParsedInst->Binary + 2);
        if(DispBits != 0)
        {
            char DispBuffer[MAX_STRING_LEN] = {0};
            bool IsNegative = (DispBits < 0);
            bool IsSigned = true;

            if(IsNegative)
            {
                strncat(RorMBuffer, " - ", 3);
                GetIntAsString_16(DispBits, DispBuffer, IsSigned);
                strncat( RorMBuffer, ((char *)(DispBuffer + 1)), (MAX_STRING_LEN - strlen(RorMBuffer)) );
            }
            else
            {
                strncat(RorMBuffer, " + ", 3);
                GetIntAsString_16(DispBits, DispBuffer, IsSigned);
                strncat( RorMBuffer, DispBuffer, (MAX_STRING_LEN - strlen(RorMBuffer)) );
            }
        }
    }
    else 
    {
        // Error
        Debug_OutputErrorMessage("Failed to parse mod field");
        exit(1);
    }
    
    strncat(RorMBuffer, "]", 1);
}

bool
CheckIfArithmetic(u8 OpcodeEnum)
{
    if(
            (OpcodeEnum == ADD) ||
            (OpcodeEnum == ADC) ||
            (OpcodeEnum == SUB) ||
            (OpcodeEnum == SBB) ||
            (OpcodeEnum == CMP)   )
    {
        return(true);
    }
    else
    {
        return(false);
    }
}


void
ReadImmField(struct parsed_inst *ParsedInst, u8 *ImmBits, char *ImmBuffer, bool CareAboutSignExtend, bool IsSigned)
{
    if(ParsedInst->IsWord)
    {
        u16 ImmValue = 0;
        if(CareAboutSignExtend)
        {
            bool IsSignExtended = (bool)(ParsedInst->Binary[0] & 0x02);
            if(IsSignExtended)
            {
                u8 Temp_u8 = *ImmBits;
                ImmValue = (u16)Temp_u8;
            }
            else
            {
                ImmValue = *(u16 *)ImmBits;
            }
        }
        else
        {
            ImmValue = *(u16 *)ImmBits;
        }
        GetIntAsString_16(ImmValue, ImmBuffer, IsSigned);
    }
    else
    {
        u8 ImmValue = *(u8 *)ImmBits;
        GetIntAsString_8(ImmValue, ImmBuffer, IsSigned);
    }
}

void
InOutDecode(struct decoded_inst *DecodedInst)
{
    // IN
    //      Fixed port: [1110 010w] [data-8]
    //      Variable port: [1110 110w] (assume d register stores port address)

    // OUT
    //      Fixed port: [1110 011w] [data-8]
    //      Variable port: [1110 111w] (assume d register stores port address)

    struct parsed_inst ParsedInst = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];
    char AccumBuffer[MAX_STRING_LEN] = {0};
    char PortBuffer[MAX_STRING_LEN] = {0};

    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.IsWord = (bool)(ByteOne & 0x01);
    bool IsVariablePort = (bool)(ByteOne & 0x08);

    DecodedInst->Size = 1;
    if(ParsedInst.IsWord)
    {
        strncpy(AccumBuffer, "ax", 2);
    }
    else
    {
        strncpy(AccumBuffer, "al", 2);
    }
    
    if(IsVariablePort)
    {
        strncpy(PortBuffer, "dx", 2);
    }
    else
    {
        DecodedInst->Size = 2;
        bool IsSigned = false;
        GetIntAsString_8(DecodedInst->Binary[1], PortBuffer, IsSigned);
    }

    if(DecodedInst->OpcodeEnum == IN_)
    {
        strncpy(DecodedInst->OperandOne, AccumBuffer, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandTwo, PortBuffer, MAX_STRING_LEN);
    }
    else
    {
        strncpy(DecodedInst->OperandOne, PortBuffer, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandTwo, AccumBuffer, MAX_STRING_LEN);
    }
}

void
RepeatDecode(struct decoded_inst *DecodedInst)
{
    // move
    // compare
    // scan
    // load
    // store
    enum
    {
        MOVSB = 0xA4,
        MOVSW = 0xA5,
        CMPSB = 0xA6,
        CMPSW = 0xA7,
        STOSB = 0xAA,
        STOSW = 0xAB,
        LODSB = 0xAC,
        LODSW = 0xAD,
        SCASB = 0xAE,
        SCASW = 0xAF,
    } StringOpEnums;

    DecodedInst->Size = 2;

    char *StringInst = NULLPTR;
    switch(DecodedInst->Binary[1])
    {
        case MOVSB:
        {
            StringInst = "movsb";
        } break;

        case MOVSW:
        {
            StringInst = "movsw";
        } break;
        
        case CMPSB:
        {
            StringInst = "cmpsb";
        } break;

        case CMPSW:
        {
            StringInst = "cmpsw";
        } break;

        case STOSB:
        {
            StringInst = "stosb";
        } break;

        case STOSW:
        {
            StringInst = "stosw";
        } break;

        case LODSB:
        {
            StringInst = "lodsb";
        } break;

        case LODSW:
        {
            StringInst = "lodsw";
        } break;

        case SCASB:
        {
            StringInst = "scasb";
        } break;

        case SCASW:
        {
            StringInst = "scasw";
        } break;

        default:
        {
            Debug_OutputErrorMessage("Failed to find matching string instruction for repeat instruction"); 
            exit(1);
        } break;
    }

    strncpy(DecodedInst->OperandOne, StringInst, MAX_STRING_LEN);
}

void
CallJumpDecode(struct decoded_inst *DecodedInst)
{
    // Direct within segment
    //      [1110 1000] [IP-INC-LO] [IP-INC-HI]
    //
    // Direct intersegment
    //      [1001 1010] [IP-lo] [IP-hi]
    //                 [CS-lo] [CS-hi]

    struct parsed_inst ParsedInst = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);
    ParsedInst.IsWord = true;
    ParsedInst.Binary = DecodedInst->Binary;

    if(ByteOne == 0xFF)
    {
        ReadRorMField(&ParsedInst, DecodedInst->OperandOne);
        DecodedInst->Size = 2;
        if( (ParsedInst.Mod == MEM_MODE_NO_DISP) && (ParsedInst.RorM == 0x06) )
        {
            // Direct address
            DecodedInst->Size = 4;
        }
        else if(ParsedInst.Mod == MEM_MODE_DISP_8)
        {
            DecodedInst->Size += 1;
        }
        else if(ParsedInst.Mod == MEM_MODE_DISP_16)
        {
            DecodedInst->Size += 2;
        }
    }
    else
    {
        Debug_OutputErrorMessage("call expression does not have 0xFF as byte one, so no codepath to decode it yet"); 
        exit(1);
    }
}

bool
IsThisALabelInstruction(u8 OpcodeEnum)
{
    u8 LabelInstructions[] = 
    {
        JE, JL, JLE, JB, JBE, JP, JO, JS, JNE, JNL, 
        JG, JNB, JA, JNP, JNO, JNS, LOOP, LOOPZ, LOOPNZ, JCXZ
    };

    for(int i = 0; i < sizeof(LabelInstructions); ++i)
    {
        if(LabelInstructions[i] == OpcodeEnum)
        {
            return(true);
        }
    }
    return(false);
}

void
DirectAddressMovDecode(struct decoded_inst *DecodedInst)
{
    struct parsed_inst ParsedInst = {0};
    char DirectAddress[MAX_STRING_LEN] = {0};
    char Reg[MAX_STRING_LEN] = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    ParsedInst.IsWord = (bool)(ByteOne & 0x01);

    bool IsWrite = (bool)((ByteOne & 0x02) >> 1);

    DecodedInst->Size = 3;

    if(ParsedInst.IsWord)
    {
        strncpy(Reg, "ax", 2);
    }
    else
    {
        strncpy(Reg, "al", 2);
    }

    u16 DirectAddressBytes = *(u16 *)(DecodedInst->Binary + 1);
    strncat(DirectAddress, "[", MAX_STRING_LEN);
    bool IsSigned = false;
    GetIntAsString_16(DirectAddressBytes, DirectAddress, IsSigned);
    strncat(DirectAddress, "]", MAX_STRING_LEN);

    if(IsWrite)
    {
        strncpy(DecodedInst->OperandOne, DirectAddress, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandTwo, Reg, MAX_STRING_LEN);
    }
    else
    {
        strncpy(DecodedInst->OperandOne, Reg, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandTwo, DirectAddress, MAX_STRING_LEN);
    }

}

void
Group9Decode(struct decoded_inst *DecodedInst)
{
    if(DecodedInst->OpcodeEnum == MOV)
    {
        DirectAddressMovDecode(DecodedInst);
    }
    else if( (DecodedInst->OpcodeEnum == IN_) || (DecodedInst->OpcodeEnum == OUT_) )
    {
        InOutDecode(DecodedInst);
    }
    else if(DecodedInst->OpcodeEnum == REP)
    {
        RepeatDecode(DecodedInst);
    }
    else if( (DecodedInst->OpcodeEnum == CALL) || (DecodedInst->OpcodeEnum == JMP) )
    {
        CallJumpDecode(DecodedInst);
    }
    else if(DecodedInst->OpcodeEnum == RET)
    {
        DecodedInst->Size = 3;
        bool IsSigned = true;
        GetIntAsString_16( (*(u16 *)(DecodedInst->Binary + 1)), DecodedInst->OperandOne, IsSigned);
    }
    else if(IsThisALabelInstruction(DecodedInst->OpcodeEnum))
    {
        DecodedInst->Size = 2;
        strncpy(DecodedInst->OperandOne, "label", MAX_STRING_LEN);
    }
    else if(DecodedInst->OpcodeEnum == INT_)
    {
        DecodedInst->Size = 2;
        bool IsSigned = false;
        GetIntAsString_8(DecodedInst->Binary[1], DecodedInst->OperandOne, IsSigned);
    }
    else
    {
        Debug_OutputErrorMessage("No matching decode function in group 9"); 
        exit(1);
    }

        


}


// G8_SHIFT,        // shifts/rotates D0–D3
// [.... ..vw] [mod ... r/m] [disp-lo] [disp-hi]
// rcr bp, 1
// ; hex: D1 DD
// ; bin: [1101 0001] [1101 1101]
//
// shl ah, cl
// ; hex: D2 E4
// ; bin: [1101 0010] [1110 0100]
//
// ; ------------------------------------------------------------------------
// ; Group 8 — shifts/rotates with ModRM (D0–D3 by 1 or by CL)
// ; ------------------------------------------------------------------------
void
Group8Decode(struct decoded_inst *DecodedInst)
{
    struct parsed_inst ParsedInst = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];
    char DestBuffer[MAX_STRING_LEN] = {0};
    char CountBuffer[MAX_STRING_LEN] = {0};

    ParsedInst.Binary = DecodedInst->Binary;
    bool VFlag = (bool)(ByteOne & 0x02);
    ParsedInst.IsWord = (bool)(ByteOne & 0x01);
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

// [.... ..vw] [mod ... r/m] [disp-lo] [disp-hi]

    DecodedInst->Size = 2;
    if(ParsedInst.Mod != REG_MODE)
    {
        if(ParsedInst.IsWord)
        {
            strncat( DestBuffer, "word ", (MAX_STRING_LEN - strlen(DestBuffer)) );
        }
        else
        {
            strncat( DestBuffer, "byte ", (MAX_STRING_LEN - strlen(DestBuffer)) );
        }

        if( (ParsedInst.Mod == MEM_MODE_NO_DISP) && (ParsedInst.RorM == 0x06) )
        {
            // Direct address
            DecodedInst->Size = 4;
        }
        else if(ParsedInst.Mod == MEM_MODE_DISP_8)
        {
            DecodedInst->Size += 1;
        }
        else if(ParsedInst.Mod == MEM_MODE_DISP_16)
        {
            DecodedInst->Size += 2;
        }
    }

    ReadRorMField(&ParsedInst, DestBuffer);
    strncat( DecodedInst->OperandOne, DestBuffer, 
             (MAX_STRING_LEN - strlen(DecodedInst->OperandOne)) );

    if(VFlag)
    {
        strncpy(DecodedInst->OperandTwo, "cl", 2);
    }
    else
    {
        strncpy(DecodedInst->OperandTwo, "1", 1);
    }
}







// ; ------------------------------------------------------------------------
// ; Group 6 — reg encoded in opcode + immediate (B0–BF MOV r,imm)
// ; ------------------------------------------------------------------------
//
// G6_OPREG_IMM,    // [.... w reg] [data] [data] (up to two bytes for trailing imm value)
//
// mov cl, 12
// ; hex: B1 0C
// ; bin: [1011 0001] [0000 1100]
//
// mov ch, -12
// ; hex: B5 F4
// ; bin: [1011 0101] [1111 0100]
void
Group6Decode(struct decoded_inst *DecodedInst)
{
    struct parsed_inst ParsedInst = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];
    char RegBuffer[MAX_STRING_LEN] = {0};
    char ImmBuffer[MAX_STRING_LEN] = {0};

    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.Reg = (ByteOne & 0x07);
    ParsedInst.IsWord = (bool)(ByteOne & 0x08);
    DecodedInst->Size = 2;

    LookUpReg(&ParsedInst, RegBuffer);
    u8 *ImmBits = (DecodedInst->Binary + 1);

    bool IsSigned = true;
    bool CareAboutSignExtend = false;

    if(ParsedInst.IsWord)
    {
        DecodedInst->Size += 1;
        ReadImmField(&ParsedInst, ImmBits, ImmBuffer, CareAboutSignExtend, IsSigned);
    }
    else
    {
        ReadImmField(&ParsedInst, ImmBits, ImmBuffer, CareAboutSignExtend, IsSigned);
    }

    strncpy( DecodedInst->OperandOne, RegBuffer, (MAX_STRING_LEN - strlen(RegBuffer)) );
    strncpy( DecodedInst->OperandTwo, ImmBuffer, (MAX_STRING_LEN - strlen(ImmBuffer)) );
}




// G5_OPREG_NODATA,        // [.... .reg] (no trailing immediate value)
// ; ------------------------------------------------------------------------
// ; Group 5 — reg encoded in opcode, no trailing data
// ; (INC/DEC r16, PUSH/POP r16, XCHG AX,r16, PUSH/POP Sreg)
// ; ------------------------------------------------------------------------
//
// push cx
// ; hex: 51
// ; bin: [0101 0001]
//
// xchg ax, dx
// ; hex: 92
// ; bin: [1001 0010]
//
// xchg ax, si
// ; hex: 96
// ; bin: [1001 0110]
void
Group5Decode(struct decoded_inst *DecodedInst)
{
    struct parsed_inst ParsedInst = {0};
    char *RegPtr = NULLPTR;
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    ParsedInst.Binary = DecodedInst->Binary;
    DecodedInst->Size = 1;
    u8 Top3Bits = (ByteOne & 0xE0);

    if( ((DecodedInst->OpcodeEnum == PUSH) || (DecodedInst->OpcodeEnum == POP)) && 
            (Top3Bits == 0) )
    {
        ParsedInst.Reg = ((ByteOne & 0x18) >> 3);
        RegPtr = SegRegLUT[ParsedInst.Reg];
    }
    else 
    {
        ParsedInst.Reg = (ByteOne & 0x07);
        RegPtr = WordRegLUT[ParsedInst.Reg];
    }

    if(DecodedInst->OpcodeEnum == XCHG)
    {
        strncpy(DecodedInst->OperandOne, "ax", 2);
        strncpy(DecodedInst->OperandTwo, RegPtr, 2);
    }
    else
    {
        strncpy(DecodedInst->OperandOne, RegPtr, 2);
    }
}


// G4_ACC_IMM
// [.... ...w] [data] [data]
void
Group4Decode(struct decoded_inst *DecodedInst)
{
// add ax, 1000
// ; hex: 05 E8 03
// ; bin: [0000 0101] [1110 1000] [0000 0011]

    struct parsed_inst ParsedInst = {0};
    char ImmValue[MAX_STRING_LEN] = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.IsWord = (bool)(ByteOne & 0x01);

    DecodedInst->Size = 2;
    u8 *ImmBits = (DecodedInst->Binary + 1);
    char *Reg = NULLPTR;
    bool IsSigned = true;
    bool CareAboutSignExtend = false;

    if(ParsedInst.IsWord)
    {
        DecodedInst->Size += 1;
        Reg = "ax";
        ReadImmField(&ParsedInst, ImmBits, ImmValue, CareAboutSignExtend, IsSigned);
    }
    else
    {
        Reg = "al";
        ReadImmField(&ParsedInst, ImmBits, ImmValue, CareAboutSignExtend, IsSigned);
    }

    strncpy(DecodedInst->OperandOne, Reg, 2);
    strncpy(DecodedInst->OperandTwo, ImmValue, strlen(ImmValue));
}



// [.... ...w] [mod <type> r/m] [disp-lo] [disp-hi]
void
Group3Decode(struct decoded_inst *DecodedInst)
{
// inc dh
// ; hex: FE C6
// ; bin: [1111 1110] [1100 0110]
//
// inc byte [bp + 1002]
// ; hex: FE 86 EA 03
// ; bin: [1111 1110] [1000 0110] [1110 1010] [0000 0011]

    struct parsed_inst ParsedInst = {0};
    char RorMField[MAX_STRING_LEN] = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.IsWord = (bool)(ByteOne & 0x01);
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    DecodedInst->Size = 2;

    if(ParsedInst.Mod != REG_MODE)
    {
        if(ParsedInst.IsWord)
        {
            strncat(RorMField, "word ", 5);
        }
        else
        {
            strncat(RorMField, "byte ", 5);
        }
        if(ParsedInst.Mod == MEM_MODE_DISP_8)
        {
            DecodedInst->Size += 1;
        }
        else if(ParsedInst.Mod == MEM_MODE_DISP_16)
        {
            DecodedInst->Size += 2;
        }
    }

    ReadRorMField(&ParsedInst, RorMField);

    strncpy( DecodedInst->OperandOne, RorMField, (MAX_STRING_LEN - strlen(RorMField)) );

}

void
Group2Decode(struct decoded_inst *DecodedInst)
{
    struct parsed_inst ParsedInst = {0};
    char RorMField[MAX_STRING_LEN] = {0};
    char ImmField[MAX_STRING_LEN] = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.IsWord = (bool)(ByteOne & 0x01);
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    DecodedInst->Size = 3;

    ReadRorMField(&ParsedInst, RorMField);
    if(ParsedInst.Mod != REG_MODE)
    {
        if(ParsedInst.IsWord)
        {
            strncat(ImmField, "word ", 5);
            DecodedInst->Size += 1;
        }
        else
        {
            strncat(ImmField, "byte ", 5);
        }
    }

    u8 *ImmBits = (ParsedInst.Binary + 2);
    if(ParsedInst.Mod == MEM_MODE_DISP_8)
    {
        DecodedInst->Size += 1;
        ImmBits += 1;
    }
    else if(ParsedInst.Mod == MEM_MODE_DISP_16)
    {
        DecodedInst->Size += 2;
        ImmBits += 2;
    }

    bool CareAboutSignExtend = CheckIfArithmetic(DecodedInst->OpcodeEnum);
    bool IsSigned = true;
    ReadImmField(&ParsedInst, ImmBits, ImmField, CareAboutSignExtend, IsSigned);

    strncpy( DecodedInst->OperandOne, RorMField, (MAX_STRING_LEN - strlen(RorMField)) );
    strncpy( DecodedInst->OperandTwo, ImmField, (MAX_STRING_LEN - strlen(ImmField)) );
}

// G1_RM_REG = 1,   // [.... ..dw] [mod reg r/m] [disp-lo] [disp-hi]
void
Group1Decode(struct decoded_inst *DecodedInst)
{
    struct parsed_inst ParsedInst = {0};
    char RegField[MAX_STRING_LEN] = {0};
    char RorMField[MAX_STRING_LEN] = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.DestFlag = (bool)((ByteOne & 0x02) >> 1);
    ParsedInst.IsWord = (bool)(ByteOne & 0x01);
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    LookUpReg(&ParsedInst, RegField);
    ReadRorMField(&ParsedInst, RorMField);
    
    DecodedInst->Size = 2;
    if( (ParsedInst.Mod == MEM_MODE_NO_DISP) && (ParsedInst.RorM == DIRECT_ADDRESS) )
    {
        DecodedInst->Size = 4;
    }
    else if(ParsedInst.Mod == MEM_MODE_DISP_8)
    {
        DecodedInst->Size += 1;
    }
    else if(ParsedInst.Mod == MEM_MODE_DISP_16)
    {
        DecodedInst->Size += 2;
    }

    if(ParsedInst.DestFlag)
    {
        strncpy(DecodedInst->OperandOne, RegField, strlen(RegField));
        strncpy(DecodedInst->OperandTwo, RorMField, strlen(RorMField));
    }
    else
    {
        strncpy(DecodedInst->OperandOne, RorMField, strlen(RorMField));
        strncpy(DecodedInst->OperandTwo, RegField, strlen(RegField));
    }
}



void
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
                DecodedInst->DecodeGroup = 1
                switch(IHaveMetTheDistinguisher)
                {
                    case 0x00: // 000
                        {
                            DecodedInst->OpcodeEnum = ADD;
                        } break;

                    case 0x08: // 001
                        {
                            DecodedInst->OpcodeEnum = OR;
                        } break;

                    case 0x10: // 010
                        {
                            DecodedInst->OpcodeEnum = ADC;
                        } break;

                    case 0x18: // 011
                        {
                            DecodedInst->OpcodeEnum = SBB;
                        } break;

                    case 0x20: // 100
                        {
                            DecodedInst->OpcodeEnum = AND;
                        } break;

                    case 0x28: // 101
                        {
                            DecodedInst->OpcodeEnum = SUB;
                        } break;

                    case 0x30: // 110
                        {
                            DecodedInst->OpcodeEnum = XOR;
                        } break;

                    case 0x38: // 111
                        {
                            DecodedInst->OpcodeEnum = CMP;
                        } break;

                    default:
                        {
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0x80 or 0x81:\n    ADD, OR, ADC, SBB, AND, SUB, XOR, or CMP");
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
                            DecodedInst->OpcodeEnum = ADD;
                        } break;

                    case 0x10: // 010
                        {
                            DecodedInst->OpcodeEnum = ADC;
                        } break;

                    case 0x18: // 011
                        {
                            DecodedInst->OpcodeEnum = SBB;
                        } break;

                    case 0x28: // 101
                        {
                            DecodedInst->OpcodeEnum = SUB;
                        } break;

                    case 0x38: // 111
                        {
                            DecodedInst->OpcodeEnum = CMP;
                        } break;

                    default:
                        {
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0x82:\n    ADD, ADC, SBB, SUB or CMP");
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
                            DecodedInst->OpcodeEnum = ADD;
                        } break;

                    case 0x10: // 010
                        {
                            DecodedInst->OpcodeEnum = ADC;
                        } break;

                    case 0x18: // 011
                        {
                            DecodedInst->OpcodeEnum = SBB;
                        } break;

                    case 0x28: // 101
                        {
                            DecodedInst->OpcodeEnum = SUB;
                        } break;

                    case 0x38: // 111
                        {
                            DecodedInst->OpcodeEnum = CMP;
                        } break;

                    default:
                        {
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0x83:\n    ADD, ADC, SBB, SUB or CMP");
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
                            DecodedInst->OpcodeEnum = ROL;
                        } break;

                    case 0x08: // 001
                        {
                            DecodedInst->OpcodeEnum = ROR;
                        } break;

                    case 0x10: // 010
                        {
                            DecodedInst->OpcodeEnum = RCL;
                        } break;

                    case 0x18: // 011
                        {
                            DecodedInst->OpcodeEnum = RCR;
                        } break;

                    case 0x20: // 100
                        {
                            DecodedInst->OpcodeEnum = SHL;
                        } break;

                    case 0x28: // 101
                        {
                            DecodedInst->OpcodeEnum = SHR;
                        } break;

                    case 0x38: // 111
                        {
                            DecodedInst->OpcodeEnum = SAR;
                        } break;

                    default:
                        {
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0xD0, 0xD1, 0xD2, 0xD3:\n    ROL, ROR, RCL, RCR, TBD_SHL, SHR, or SAR");
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
                            DecodedInst->OpcodeEnum = TEST;
                        } break;

                    case 0x10: // 010
                        {
                            DecodedInst->OpcodeEnum = NOT;
                        } break;

                    case 0x18: // 011
                        {
                            DecodedInst->OpcodeEnum = NEG;
                        } break;

                    case 0x20: // 100
                        {
                            DecodedInst->OpcodeEnum = MUL;
                        } break;

                    case 0x28: // 101
                        {
                            DecodedInst->OpcodeEnum = IMUL;
                        } break;

                    case 0x30: // 110
                        {
                            DecodedInst->OpcodeEnum = DIV;
                        } break;

                    case 0x38: // 111
                        {
                            DecodedInst->OpcodeEnum = IDIV;
                        } break;

                    default:
                        { 
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0xF6 or 0xF7:\n   TEST, NOT, NEG, MUL, IMUL, DIV or IDIV"); 
                            exit(1);
                        }
                }
            }

        case 0xFE:
            {
                switch(IHaveMetTheDistinguisher)
                {
                    case 0x00: // 000
                        {
                            DecodedInst->OpcodeEnum = INC;
                        } break;

                    case 0x08: // 010
                        {
                            DecodedInst->OpcodeEnum = DEC;
                        } break;

                    default:
                        {
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0xFE:\n   INC or DEC"); 
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
                            DecodedInst->OpcodeEnum = INC;
                        } break;

                    case 0x08: // 001
                        {
                            DecodedInst->OpcodeEnum = DEC;
                        } break;

                    case 0x10: // 010
                    case 0x18: // 011
                        {
                            DecodedInst->OpcodeEnum = CALL;
                        } break;

                    case 0x20: // 100
                    case 0x28: // 101
                        {
                            DecodedInst->OpcodeEnum = JMP;
                        } break;

                    case 0x30: // 110
                        {
                            DecodedInst->OpcodeEnum = PUSH;
                        } break;

                    default:
                        {
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0xFF:\n   INC, DEC, CALL, JMP or PUSH"); 
                            exit(1);
                        } break;
                }
            } break;

        default:
            {
                Debug_OutputErrorMessage("Failed to decode extended opcode");
                exit(1);
            } break;
    }
}

