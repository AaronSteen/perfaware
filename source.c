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
Debug_PrintCurrentStatus(struct decoded_inst *DecodedInst)
{
    // Print the instruction we just decoded
    char DecodedInstBuf[MAX_STRING_LEN] = {0};
    sprintf_s(DecodedInstBuf, sizeof(DecodedInstBuf), "\n  *** %s %s, %s ***  \n\n", 
                DecodedInst->Mnemonic, DecodedInst->OperandOne, DecodedInst->OperandTwo);
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
GetIntAsString_8(u8 IntBits, char *StrBuffer)
{
    char Temp[MAX_STRING_LEN] = {0};
    sprintf_s(Temp, MAX_STRING_LEN, "%d", IntBits);
    size_t DoNotOverflow = (MAX_STRING_LEN - strlen(StrBuffer));
    strncat(StrBuffer, Temp, DoNotOverflow);
}


void
GetIntAsString_16(u16 IntBits, char *StrBuffer)
{
    char Temp[MAX_STRING_LEN] = {0};
    sprintf_s(Temp, MAX_STRING_LEN, "%d", IntBits);
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
            GetIntAsString_16(DispBits, RorMBuffer);
        }
        else
        {
            LookUpEffectiveAddress(ParsedInst, RorMBuffer);
        }
    }
    else if(ParsedInst->Mod == MEM_MODE_DISP_8)
    {
        LookUpEffectiveAddress(ParsedInst, RorMBuffer);
        u8 DispBits = ParsedInst->Binary[2];
        if(DispBits > 0)
        {
            strncat(RorMBuffer, " + ", 3);
            char DispBuffer[MAX_STRING_LEN] = {0};
            GetIntAsString_8(DispBits, DispBuffer);
            strncat(RorMBuffer, DispBuffer, MAX_STRING_LEN);
        }
    }

    else if(ParsedInst->Mod == MEM_MODE_DISP_16)
    {
        LookUpEffectiveAddress(ParsedInst, RorMBuffer);
        u16 DispBits = *(u16 *)(ParsedInst->Binary + 2);
        if(DispBits > 0)
        {
            strncat(RorMBuffer, " + ", 3);
            char DispBuffer[MAX_STRING_LEN] = {0};
            GetIntAsString_16(DispBits, DispBuffer);
            strncat(RorMBuffer, DispBuffer, MAX_STRING_LEN);
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

void
ReadImmField(u8 *ImmValueBits, bool IsWord, char *ImmBuffer)
{
    if(IsWord)
    {
        u16 ImmValue = *(u16 *)ImmValueBits;
        GetIntAsString_16(ImmValue, ImmBuffer);
    }
    else
    {
        u8 ImmValue = *(u8 *)ImmValueBits;
        GetIntAsString_8(ImmValue, ImmBuffer);
    }
}

// imm -> r/m
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

    ReadRorMField(&ParsedInst, RorMField);
    if(ParsedInst.Mod != REG_MODE)
    {
        if(ParsedInst.IsWord)
        {
            strncat(ImmField, "word ", 5);
        }
        else
        {
            strncat(ImmField, "byte ", 5);
        }
    }

    DecodedInst->Size = 2;
    u8 *ImmBytes = (ParsedInst.Binary + 2);
    if(ParsedInst.Mod == MEM_MODE_DISP_8)
    {
        DecodedInst->Size += 1;
        ImmBytes += 1;
    }
    else if(ParsedInst.Mod == MEM_MODE_DISP_16)
    {
        DecodedInst->Size += 2;
        ImmBytes += 2;
    }

    ReadImmField(ImmBytes, ParsedInst.IsWord, ImmField);

    strncpy( DecodedInst->OperandOne, RorMField, (MAX_STRING_LEN - strlen(RorMField)) );
    strncpy( DecodedInst->OperandTwo, ImmField, (MAX_STRING_LEN - strlen(ImmField)) );
}

// r/m <-> reg
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
    if(ParsedInst.Mod == MEM_MODE_DISP_8)
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
                            return(SHL);
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
                            return(INC);
                        } break;

                    case 0x08: // 010
                        {
                            return(DEC);
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

void
Dispatch(struct decoded_inst *DecodedInst)
{
#if 0
    switch(DecodedInst->OpcodeEnum)
    {
        case(MOV):
            enum 
            {
                MOV_MEMorREG_TOorFROM_REG = 0x88,
                MOV_IMM_TO_REGorMEM = 0xC6,
                MOV_IMM_TO_REG = 0xB0,
                MOV_MEM_TO_ACCUM = 0xA0,
                MOV_ACCUM_TO_MEM = 0xA2,
                MOV_REGorMEM_TO_SEGREG = 0x8E,
                MOV_SEGREG_TO_REGorMEM = 0x8C,
            };
            // if( DecodedInst->Binary[0] );
            //     Group1Decode(DecodedInst);
            } break;

        case(MOV):
            {
                if( (0xB1 <= DecodedInst->OpcodeEnum) &&
                        (DecodedInst->OpcodeEnum <= 0xBA) )
                {
                    Group5Decode(DecodedInst);
                }
                else
                {
                    Group1Decode(DecodedInst);
                }
            } break;
        case(PUSH):
            {
                DecodePush(DecodedInst);
            } break;

        case(POP):
            {
                DecodePop(DecodedInst);
            } break;

        case(XCHG):
        case(NOP):
            {
                DecodeXchg(DecodedInst);
            } break;

        case(IN_):
        case(OUT_):
            {
                DecodeInOrOut(DecodedInst);
            } break;

        case(LEA):
        case(LDS):
        case(LES):
            {
                DecodeLoad(DecodedInst);
            } break;

        case(ADD):
        case(ADC):
        case(SUB):
        case(SBB):
        case(CMP):
            {
                DecodeArithmetic(DecodedInst);
            } break;

        case(INC):
        case(DEC):
            {
                DecodeIncOrDec(DecodedInst);
            } break;

        case(XLAT): 
        case(LAHF):
        case(SAHF):
        case(PUSHF):
        case(POPF):
        case(AAA):
        case(DAA):
        case(AAS):
        case(DAS):
        case(CBW):
        case(CWD):
        case(INTO):
        case(IRET):
        case(CLC):
        case(CMC):
        case(STC):
        case(CLD):
        case(STD):
        case(CLI):
        case(STI):
        case(HLT):
        case(WAIT):
        case(RET):
            {
                DecodedInst->Size = 1;
                return;
            } break;

        case(AAM):
        case(AAD):
            {
                DecodedInst->Size = 2;
                return;
            } break;

        case(NEG):
        case(NOT):
            
            {
                DecodeGroup3(DecodedInst);
                return;
            } break;

        case(MUL):
        case(IMUL):
        case(DIV):
        case(IDIV):
            {
                DecodeGroup3(DecodedInst);
                return;
            } break;

        case(SHL):
        case(SHR):
        case(SAR):
        case(ROL):
        case(ROR):
        case(RCL):
        case(RCR):
            {
                DecodeGroup3(DecodedInst);

                BOOL VFlag = ((DecodedInst->Binary[0] & 0x02) >> 1);
                if(VFlag)
                {
                    SteenCopy(DecodedInst->OperandTwo, "cl", 2);
                }
                else
                {
                    SteenCopy(DecodedInst->OperandTwo, "1", 1);
                }
                return;
            } break;


#endif
        default:
            {
                // Error
                Debug_PrintCurrentStatus(DecodedInst);
                Debug_OutputErrorMessage("No matching dispatch function");
                exit(1);
            } break;
    }
}

