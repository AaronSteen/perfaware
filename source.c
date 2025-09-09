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
SafeCopy(void)
{
    //stub
}

void
Group1Decode(struct decoded_inst *DecodedInst)
{
    struct parsed_inst ParsedInst = {0};
    char *RegField = NULLPTR;
    char *RorMField = NULLPTR;
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.DestFlag = (bool)((ByteOne & 0x02) >> 1);
    ParsedInst.IsWord = (bool)(ByteOne & 0x01);
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    DecodedInst->Size = 2;

    if(ParsedInst.IsWord)
    {
        RegField = WordRegLUT[ByteTwo];
    }
    else
    {
        RegField = ByteRegLUT[ByteTwo];
    }

    if(ParsedInst.Mod == REG_MODE)
    {
        if(ParsedInst.IsWord)
        {
            RorMField = WordRegLUT[ByteTwo];
        }
        else
        {
            RorMField = ByteRegLUT[ByteTwo];
        }
    }
    else
    {
        // TODO: Check for direct address.

        RorMField = ReadRorMField(ParsedInst.RorM);

        if(ParsedInst.Mod == MEM_MODE_DISP_8)
        {
            DecodedInst.Size += 1;
            Read8BitDisp();
        }
        else if(ParsedInst.Mod == MEM_MODE_DISP_16)
        {
            DecodedInst.Size += 2;
            Read16BitDisp();
        }
        
        CloseBracket(RorMField);
    }

    if(ParsedInst.DestFlag)
    {

        
        DecodedInst->OperandOne
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
    switch(DecodedInst->OpcodeEnum)
    {
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



        default:
            {
                // Error
                Debug_PrintCurrentStatus(DecodedInst);
                Debug_OutputErrorMessage("No matching dispatch function");
                exit(1);
            } break;
    }
}

