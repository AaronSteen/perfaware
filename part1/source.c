#include "common.h"
#include "tables.c"
#include "assert.h"

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

// char *
// Debug_GetRegNameAsString(int i)
// {
//
//     // We have to convert the index of the register whose name we want to print
//     //      from our logical, alphabetical, linear indexing scheme to the
//     //      psycho octal codes 8086 uses, which we have previously enumed in common.h
//     if(IsOtherReg)
//     {
//         OctalIndices = {[
//
//
//
//     }
//     int OctalIndices[] = {AX, BX, CX, DX, SP, BP, SI, DI, ES, CS, SS, DS, IP, FLAGS};
//     int OctalIndex = OctalIndices[i];
//     // Then we use the octal index to look up the string for the register's name,
//     //      in the same way we do during decoding.
//     return(WordRegLUT[OctalIndex]);
// }

void
Debug_PrintUpdatedRegisterState(struct decoded_inst *DecodedInst, union registers *NewRegisters,
                                   union registers *OldRegisters)
{
    char *SpecialCombinedWordRegLUT[] = 
            {"ax", "bx", "cx", "dx", "sp", "bp", "si", "di", "es", "cs", "ss", "ds", "ip", "flags"};
    // Print the decoded instruction
    printf("%s %s, %s       ; ", DecodedInst->Mnemonic, DecodedInst->OperandOneStr, DecodedInst->OperandTwoStr);
    bool NeedSpacing = false;
    for(int i = 0; i < NUMBER_OF_WORD_SIZED_REGISTERS; i++)
    {
        if(NewRegisters->WordRegisters[i] != OldRegisters->WordRegisters[i])
        {
            // map our logical 8086 register layout in memory to the psycho
            //      8086 octal encoding since they are not the same
            char *RegName = SpecialCombinedWordRegLUT[i];
            if(NeedSpacing)
            {
                printf("                ; ");
            }
            printf("%8s: 0x%02x->0x%02x\n", RegName, OldRegisters->WordRegisters[i], NewRegisters->WordRegisters[i]);
            NeedSpacing = true;
        }
    }
}
void
Debug_PrintFinalRegisterState(union registers *Registers)
{
    char *SpecialCombinedWordRegLUT[] = 
            {"ax", "bx", "cx", "dx", "sp", "bp", "si", "di", "es", "cs", "ss", "ds", "ip", "flags"};

    printf("Final registers:\n");

    for(int i = 0; i < NUMBER_OF_WORD_SIZED_REGISTERS; i++)
    {
        if(Registers->WordRegisters[i])
        {
            char *RegName = SpecialCombinedWordRegLUT[i];
            u16 Value = Registers->WordRegisters[i];
            printf("%8s: ", RegName);
            printf("0x%04x (%u)", Value, Value);
            printf("\n");
        }
    }
}

void
Debug_PrintCurrentStatus(struct decoded_inst *DecodedInst, int InstStreamIdx)
{
    // Print the instruction we just decoded
    char DecodedInstBuf[MAX_STRING_LEN] = {0};
    sprintf_s(DecodedInstBuf, sizeof(DecodedInstBuf), "\n         Idx %d         \n", InstStreamIdx);
    OutputDebugStringA(DecodedInstBuf);
    sprintf_s(DecodedInstBuf, sizeof(DecodedInstBuf), "\n  *** %s %s%s %s ***  \n\n", 
                DecodedInst->Mnemonic, DecodedInst->OperandOneStr, 
                ( (DecodedInst->OperandTwoStr[0] != '\0') ? "," : "" ), DecodedInst->OperandTwoStr);
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

struct inst_stream
Win32_LoadInstStream(HANDLE FileHandle)
{
    struct inst_stream InstStream = {0};

    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        GetFileSizeEx(FileHandle, &InstStream.StreamSize);
        if(InstStream.StreamSize.QuadPart)
        {
            HANDLE HeapHandle = GetProcessHeap();
            if(HeapHandle)
            {
                InstStream.Start = (u8 *)HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, 
                        (SIZE_T)InstStream.StreamSize.LowPart);
                if(InstStream.Start)
                {
                    DWORD BytesRead = 0;
                    bool Result = ReadFile(FileHandle, InstStream.Start, 
                            InstStream.StreamSize.LowPart, &BytesRead, 0);
                    if((BytesRead == InstStream.StreamSize.LowPart) && Result)
                    {
                        InstStream.DoNotCrossThisLine = ((u8 *)InstStream.Start +
                                InstStream.StreamSize.LowPart);
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

    return InstStream;
}

void
LookUpReg(struct decoded_inst *DecodedInst, char *StrBuffer)
{
    int DoNotOverflow = MAX_STRING_LEN - strlen(StrBuffer);
    if(DecodedInst->IsWord)
    {
        // look up the string corresponding to the register
        strncat(StrBuffer, WordRegLUT[DecodedInst->Reg], DoNotOverflow);
    }
    else
    {
        // as above but for byte-width registers
        strncat(StrBuffer, ByteRegLUT[DecodedInst->Reg], DoNotOverflow);
    }
}

void
LookUpEffectiveAddress(struct decoded_inst *DecodedInst, char *StrBuffer)
{
    if(DecodedInst->IsWord)
    {
        strncat(StrBuffer, EffectiveAddressLUT[DecodedInst->RorM], MAX_STRING_LEN);
    }
    else
    {
        strncat(StrBuffer, EffectiveAddressLUT[DecodedInst->RorM], MAX_STRING_LEN);
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
ReadRorMField(struct decoded_inst *DecodedInst, char *RorMBuffer)
{
    if(DecodedInst->Mod == REG_MODE)
    {
        int DoNotOverflow = MAX_STRING_LEN - strlen(RorMBuffer);
        if(DecodedInst->IsWord)
        {
            strncat(RorMBuffer, WordRegLUT[DecodedInst->RorM], DoNotOverflow);
        }
        else
        {
            strncat(RorMBuffer, ByteRegLUT[DecodedInst->RorM], DoNotOverflow);
        }
        return;
    }

    // Else it's a memory operation
    if(DecodedInst->Mod == MEM_MODE_NO_DISP)
    {
        if(DecodedInst->RorM == DIRECT_ADDRESS)
        {
            u16 DispBits = *(u16 *)(DecodedInst->Binary + 2);
            strncat(RorMBuffer, "[", MAX_STRING_LEN);
            GetIntAsString_16(DispBits, RorMBuffer, false);
        }
        else
        {
            LookUpEffectiveAddress(DecodedInst, RorMBuffer);
        }
    }
    else if(DecodedInst->Mod == MEM_MODE_DISP_8)
    {
        LookUpEffectiveAddress(DecodedInst, RorMBuffer);
        s8 DispBits = *(s8 *)(DecodedInst->Binary + 2);
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
    else if(DecodedInst->Mod == MEM_MODE_DISP_16)
    {
        LookUpEffectiveAddress(DecodedInst, RorMBuffer);
        s16 DispBits = *(s16 *)(DecodedInst->Binary + 2);
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

bool
CheckIfLogical(u8 OpcodeEnum)
{
    if(
            (OpcodeEnum == AND) ||
            (OpcodeEnum == TEST) ||
            (OpcodeEnum == OR) ||
            (OpcodeEnum == XOR) )
    {
        return(true);
    }
    else
    {
        return(false);
    }
}


void
ReadImmField(struct decoded_inst *DecodedInst, u8 *ImmBits, char *ImmBuffer, bool CareAboutSignExtend, bool IsSigned)
{
    if(DecodedInst->IsWord)
    {
        u16 ImmValue = 0;
        if(CareAboutSignExtend)
        {
            bool IsSignExtended = (bool)(DecodedInst->Binary[0] & 0x02);
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

    
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];
    char AccumBuffer[MAX_STRING_LEN] = {0};
    char PortBuffer[MAX_STRING_LEN] = {0};

    
    DecodedInst->IsWord = (bool)(ByteOne & 0x01);
    bool IsVariablePort = (bool)(ByteOne & 0x08);

    DecodedInst->Size = 1;
    if(DecodedInst->IsWord)
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
        strncpy(DecodedInst->OperandOneStr, AccumBuffer, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandTwoStr, PortBuffer, MAX_STRING_LEN);
    }
    else
    {
        strncpy(DecodedInst->OperandOneStr, PortBuffer, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandTwoStr, AccumBuffer, MAX_STRING_LEN);
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

    strncpy(DecodedInst->OperandOneStr, StringInst, MAX_STRING_LEN);
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

    
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];
    DecodedInst->Mod = ((ByteTwo & MOD_FIELD) >> 6);
    DecodedInst->RorM = (ByteTwo & R_OR_M_FIELD);
    DecodedInst->IsWord = true;

    if(ByteOne == 0xFF)
    {
        ReadRorMField(DecodedInst, DecodedInst->OperandOneStr);
        DecodedInst->Size = 2;
        if( (DecodedInst->Mod == MEM_MODE_NO_DISP) && (DecodedInst->RorM == 0x06) )
        {
            // Direct address
            DecodedInst->Size = 4;
        }
        else if(DecodedInst->Mod == MEM_MODE_DISP_8)
        {
            DecodedInst->Size += 1;
        }
        else if(DecodedInst->Mod == MEM_MODE_DISP_16)
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
    char DirectAddressBuffer[MAX_STRING_LEN] = {0};
    char RegBuffer[MAX_STRING_LEN] = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    DecodedInst->IsWord = (bool)(ByteOne & 0x01);
    DecodedInst->DestFlag = (bool)((ByteOne & 0x02) >> 1);

    DecodedInst->Size = 3;

    if(DecodedInst->IsWord)
    {
        strncpy(RegBuffer, "ax", 2);
    }
    else
    {
        strncpy(RegBuffer, "al", 2);
    }

    strncat(DirectAddressBuffer, "[", MAX_STRING_LEN);
    u16 DirectAddressBytes = *(u16 *)(DecodedInst->Binary + 1);
    bool IsSigned = false;
    GetIntAsString_16(DirectAddressBytes, DirectAddressBuffer, IsSigned);
    strncat(DirectAddressBuffer, "]", MAX_STRING_LEN);

    if(DecodedInst->DestFlag)
    {
        strncpy(DecodedInst->OperandOneStr, DirectAddressBuffer, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandTwoStr, RegBuffer, MAX_STRING_LEN);
    }
    else
    {
        strncpy(DecodedInst->OperandOneStr, RegBuffer, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandTwoStr, DirectAddressBuffer, MAX_STRING_LEN);
    }

}

// G1_RM_REG = 1,   // [.... ..dw] [mod reg r/m] [disp-lo] [disp-hi]
void
Group1Decode(struct decoded_inst *DecodedInst)
{
    
    char RegField[MAX_STRING_LEN] = {0};
    char RorMField[MAX_STRING_LEN] = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    
    DecodedInst->DestFlag = (bool)((ByteOne & 0x02) >> 1);
    DecodedInst->IsWord = (bool)(ByteOne & 0x01);
    DecodedInst->Mod = ((ByteTwo & MOD_FIELD) >> 6);
    DecodedInst->Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    DecodedInst->RorM = (ByteTwo & R_OR_M_FIELD);

    LookUpReg(DecodedInst, RegField);
    ReadRorMField(DecodedInst, RorMField);

    if(DecodedInst->Mod == REG_MODE)
    {
        DecodedInst->Size = 2;

        // set operands to integer reg values for use by DoInstruction function
        if(DecodedInst->DestFlag)
        {
            DecodedInst->OperandOne = (u16)DecodedInst->Reg;
            DecodedInst->OperandTwo = (u16)DecodedInst->RorM;
        }
        else
        {
            DecodedInst->OperandTwo = (u16)DecodedInst->Reg;
            DecodedInst->OperandOne = (u16)DecodedInst->RorM;
        }

    }
    else if(DecodedInst->Mod == MEM_MODE_NO_DISP)
    {
        DecodedInst->Size = 2;
        if(DecodedInst->RorM == DIRECT_ADDRESS)
        {
            DecodedInst->Size = 4;
        }
    }
    else if(DecodedInst->Mod == MEM_MODE_DISP_8)
    {
        DecodedInst->Size = 3;
    }
    else if(DecodedInst->Mod == MEM_MODE_DISP_16)
    {
        DecodedInst->Size = 4;
    }

    if(DecodedInst->DestFlag)
    {
        strncpy(DecodedInst->OperandOneStr, RegField, strlen(RegField));
        strncpy(DecodedInst->OperandTwoStr, RorMField, strlen(RorMField));
    }
    else
    {
        strncpy(DecodedInst->OperandOneStr, RorMField, strlen(RorMField));
        strncpy(DecodedInst->OperandTwoStr, RegField, strlen(RegField));
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
        GetIntAsString_16( (*(u16 *)(DecodedInst->Binary + 1)), DecodedInst->OperandOneStr, IsSigned);
    }
    else if(IsThisALabelInstruction(DecodedInst->OpcodeEnum))
    {
        DecodedInst->Size = 2;
        strncpy(DecodedInst->OperandOneStr, "label", MAX_STRING_LEN);
    }
    else if(DecodedInst->OpcodeEnum == INT_)
    {
        DecodedInst->Size = 2;
        bool IsSigned = false;
        GetIntAsString_8(DecodedInst->Binary[1], DecodedInst->OperandOneStr, IsSigned);
    }
    else if( (DecodedInst->OpcodeEnum == LEA) || (DecodedInst->OpcodeEnum == LDS) )
    {
        Group1Decode(DecodedInst);
        char Temp[MAX_STRING_LEN] = {0};
        strncpy(Temp, DecodedInst->OperandOneStr, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandOneStr, DecodedInst->OperandTwoStr, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandTwoStr, Temp, MAX_STRING_LEN);
    }
    else if(DecodedInst->OpcodeEnum == LES)
    {
        Group1Decode(DecodedInst);
        int RegCode = ( ((DecodedInst->Binary[1]) & FLEX_FIELD) >> 3 );
        strncpy(DecodedInst->OperandTwoStr, WordRegLUT[RegCode], MAX_STRING_LEN);
        char Temp[MAX_STRING_LEN] = {0};
        strncpy(Temp, DecodedInst->OperandOneStr, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandOneStr, DecodedInst->OperandTwoStr, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandTwoStr, Temp, MAX_STRING_LEN);
    }
    else if( (DecodedInst->OpcodeEnum == AAD) || (DecodedInst->OpcodeEnum == AAM) )
    {
        DecodedInst->Size = 2;
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
    
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];
    char DestBuffer[MAX_STRING_LEN] = {0};
    char CountBuffer[MAX_STRING_LEN] = {0};

    
    bool VFlag = (bool)(ByteOne & 0x02);
    DecodedInst->IsWord = (bool)(ByteOne & 0x01);
    DecodedInst->Mod = ((ByteTwo & MOD_FIELD) >> 6);
    DecodedInst->RorM = (ByteTwo & R_OR_M_FIELD);

// [.... ..vw] [mod ... r/m] [disp-lo] [disp-hi]

    DecodedInst->Size = 2;
    if(DecodedInst->Mod != REG_MODE)
    {
        if(DecodedInst->IsWord)
        {
            strncat( DestBuffer, "word ", (MAX_STRING_LEN - strlen(DestBuffer)) );
        }
        else
        {
            strncat( DestBuffer, "byte ", (MAX_STRING_LEN - strlen(DestBuffer)) );
        }

        if( (DecodedInst->Mod == MEM_MODE_NO_DISP) && (DecodedInst->RorM == 0x06) )
        {
            // Direct address
            DecodedInst->Size = 4;
        }
        else if(DecodedInst->Mod == MEM_MODE_DISP_8)
        {
            DecodedInst->Size += 1;
        }
        else if(DecodedInst->Mod == MEM_MODE_DISP_16)
        {
            DecodedInst->Size += 2;
        }
    }

    ReadRorMField(DecodedInst, DestBuffer);
    strncat( DecodedInst->OperandOneStr, DestBuffer, 
             (MAX_STRING_LEN - strlen(DecodedInst->OperandOneStr)) );

    if(VFlag)
    {
        strncpy(DecodedInst->OperandTwoStr, "cl", 2);
    }
    else
    {
        strncpy(DecodedInst->OperandTwoStr, "1", 1);
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
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];
    char RegBuffer[MAX_STRING_LEN] = {0};
    char ImmBuffer[MAX_STRING_LEN] = {0};

    
    DecodedInst->Reg = (ByteOne & 0x07);
    DecodedInst->OperandOne = DecodedInst->Reg;
    DecodedInst->IsWord = (bool)(ByteOne & 0x08);
    DecodedInst->Size = 2;

    LookUpReg(DecodedInst, RegBuffer);
    u8 *ImmBits = (DecodedInst->Binary + 1);

    bool IsSigned = false;
    bool CareAboutSignExtend = false;

    // stop. fix the way we print signed and unsigned values. print the "nicest"
    //      version; i.e., if the sign bit is set, print signed or unsigned based on which
    //      has the smaller absolute value.

    if(DecodedInst->IsWord)
    {
        DecodedInst->Size += 1;
        ReadImmField(DecodedInst, ImmBits, ImmBuffer, CareAboutSignExtend, IsSigned);
        DecodedInst->OperandTwo = *(u16 *)ImmBits;
    }
    else
    {
        ReadImmField(DecodedInst, ImmBits, ImmBuffer, CareAboutSignExtend, IsSigned);
        DecodedInst->OperandTwo = *(u8 *)ImmBits;
    }

    strncpy( DecodedInst->OperandOneStr, RegBuffer, (MAX_STRING_LEN - strlen(RegBuffer)) );
    strncpy( DecodedInst->OperandTwoStr, ImmBuffer, (MAX_STRING_LEN - strlen(ImmBuffer)) );
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
    
    char *RegPtr = NULLPTR;
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    
    DecodedInst->Size = 1;
    u8 Top3Bits = (ByteOne & 0xE0);

    if( ((DecodedInst->OpcodeEnum == PUSH) || (DecodedInst->OpcodeEnum == POP)) && 
            (Top3Bits == 0) )
    {
        DecodedInst->Reg = ((ByteOne & 0x18) >> 3);
        RegPtr = OtherRegLUT[DecodedInst->Reg];
    }
    else 
    {
        DecodedInst->Reg = (ByteOne & 0x07);
        RegPtr = WordRegLUT[DecodedInst->Reg];
    }

    if(DecodedInst->OpcodeEnum == XCHG)
    {
        strncpy(DecodedInst->OperandOneStr, "ax", 2);
        strncpy(DecodedInst->OperandTwoStr, RegPtr, 2);
    }
    else
    {
        strncpy(DecodedInst->OperandOneStr, RegPtr, 2);
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

    
    char ImmValue[MAX_STRING_LEN] = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    
    DecodedInst->IsWord = (bool)(ByteOne & 0x01);

    DecodedInst->Size = 2;
    u8 *ImmBits = (DecodedInst->Binary + 1);
    char *Reg = NULLPTR;
    bool IsSigned = true;
    bool CareAboutSignExtend = false;

    if(DecodedInst->IsWord)
    {
        DecodedInst->Size += 1;
        Reg = "ax";
        ReadImmField(DecodedInst, ImmBits, ImmValue, CareAboutSignExtend, IsSigned);
    }
    else
    {
        Reg = "al";
        ReadImmField(DecodedInst, ImmBits, ImmValue, CareAboutSignExtend, IsSigned);
    }

    strncpy(DecodedInst->OperandOneStr, Reg, 2);
    strncpy(DecodedInst->OperandTwoStr, ImmValue, strlen(ImmValue));
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

    
    char RorMField[MAX_STRING_LEN] = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    DecodedInst->IsWord = (bool)(ByteOne & 0x01);
    DecodedInst->Mod = ((ByteTwo & MOD_FIELD) >> 6);
    DecodedInst->RorM = (ByteTwo & R_OR_M_FIELD);

    DecodedInst->Size = 2;

    if(DecodedInst->Mod != REG_MODE)
    {
        if(DecodedInst->IsWord)
        {
            strncat(RorMField, "word ", 5);
        }
        else
        {
            strncat(RorMField, "byte ", 5);
        }

        if( (DecodedInst->Mod == MEM_MODE_NO_DISP) && (DecodedInst->RorM == 0x06) )
        {
            // Direct address
            DecodedInst->Size = 4;
        }

        else if(DecodedInst->Mod == MEM_MODE_DISP_8)
        {
            DecodedInst->Size += 1;
        }
        else if(DecodedInst->Mod == MEM_MODE_DISP_16)
        {
            DecodedInst->Size += 2;
        }
    }

    ReadRorMField(DecodedInst, RorMField);

    strncpy( DecodedInst->OperandOneStr, RorMField, (MAX_STRING_LEN - strlen(RorMField)) );

}

// if arithmetic:
//
//     2 bytes for instruction: [opcode sw] [mod <type> r/m] 
//
//     2 potential disp bytes if writing the immediate value to memory: [disp-lo] [disp-hi]
//
//     2 potential data bytes [data] [data] determined by the following:
//         - if SW == 00, only 1 data byte because it's an 8-bit value; nothing else to it
//         - if SW == 01, 2 data bytes because it's a 16-bit value; nothing else to it
//         - if SW == 10, invalid
//         - if SW == 11, only 1 data byte but the stored value should be 2 bytes because we take the
//                         8-bit value and sign-extend it to 16 bits before storing it in the
//                         extended (bx etc.) register
void
Group2Decode(struct decoded_inst *DecodedInst)
{
    char RorMField[MAX_STRING_LEN] = {0};
    char ImmField[MAX_STRING_LEN] = {0};
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    DecodedInst->IsWord = (bool)(ByteOne & 0x01);
    DecodedInst->Mod = ((ByteTwo & MOD_FIELD) >> 6);
    DecodedInst->RorM = (ByteTwo & R_OR_M_FIELD);

    DecodedInst->Size = 3;

    ReadRorMField(DecodedInst, RorMField);
    if(DecodedInst->Mod != REG_MODE)
    {
        if(DecodedInst->IsWord)
        {
            strncat(ImmField, "word ", 5);
            DecodedInst->Size += 1;
        }
        else
        {
            strncat(ImmField, "byte ", 5);
        }
    }

    u8 *ImmBits = (DecodedInst->Binary + 2);
    if(DecodedInst->Mod == MEM_MODE_DISP_8)
    {
        DecodedInst->Size += 1;
        ImmBits += 1;
    }
    else if(DecodedInst->Mod == MEM_MODE_DISP_16)
    {
        DecodedInst->Size += 2;
        ImmBits += 2;
    }

    bool CareAboutSignExtend;
    if ( (CareAboutSignExtend = CheckIfArithmetic(DecodedInst->OpcodeEnum)) )
    {
        // About sign extension (the S flag) and word instructions (the W flag).
        //
        //      The W flag determines the width of the LOCATION into which a value
        //      is written.
        //      
        //      The S flag determines the width of the VALUE that is written there.
        //
        //      Only add, adc, sub, sbb and cmp instructions have an S flag. In
        //              the case of these instructions, if the W flag is set,
        //              we need to check the S flag to see whether the 
        //              immediate value encoded in the instruction is 16 bits
        //              or 8 bits.
        //
        //      If the S flag IS NOT set, it means the immediate value is 16 bits
        //              (what we would describe as "the typical case"; i.e., 
        //              the encoding approach doesn't differ from other immediate instructions)
        //
        //      If the S flag IS set, it means the immediate value is 8 bits, but
        //              should be sign-extended; i.e., it should be stored in an s16 variable
        //              before being written to its 16-bit destination
        //      
        //      The switch statement below is unnecessary because the only case in which we
        //              change the codepath is in the case where S = 0 and W = 1, so we could
        //              use a single if statement instead. However, presenting it in this way makes
        //              the relationship between the S and W flags clear, and performance
        //              isn't a concern for this project right now, so we leave it in for the moment.
        //      
        u8 SWBits = (ByteOne & 0x03);
        switch(SWBits)
        {
            case 0x00: // binary 0000 0000
            {
                // 8-bit immediate value, which we already account for at the top of
                //      the function by setting the instruction size to 3,
                //      written into a destination whose width is 8-bits
                DecodedInst->Size += 0;
            } break;

            case 0x01: // binary 0000 0001
            {
                // 16-bit immediate value (instead of 8-bit immediate value above), to be
                //      written to a destination with a width of 16-bits
                DecodedInst->Size += 1;
            } break;

            case 0x03: // binary 0000 0011
            {
                // 8-bit immediate value stored in the instruction that must be
                //      sign-extended when written to the destination. As with case
                //      0x00, we already account for this at the start of the function
                DecodedInst->Size += 0;
            } break;

            default:
            {
                // We'd hit this block if the S bit was set and the W bit was not, which is
                //      nonsensical: we wouldn't sign extend a value to be 16 bits if 
                //      we were writing it into an 8-bit register.
                Debug_OutputErrorMessage("Invalid SW bits combination for arithmetic instruction");
                exit(1);

            } break;
        }
        

    }

    bool IsSigned = true;
    if(CheckIfLogical(DecodedInst->OpcodeEnum))
    {
        IsSigned = false;
    }
    ReadImmField(DecodedInst, ImmBits, ImmField, CareAboutSignExtend, IsSigned);

    strncpy( DecodedInst->OperandOneStr, RorMField, (MAX_STRING_LEN - strlen(RorMField)) );
    strncpy( DecodedInst->OperandTwoStr, ImmField, (MAX_STRING_LEN - strlen(ImmField)) );
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
        case 0x82:
        case 0x83:
        {
            DecodedInst->DecodeGroup = G2_IMM_RM;
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
                    Debug_OutputErrorMessage("Failed to decode extended opcode 0x80, 0x81, 0x82, 0x83:\n \
                            ADD, OR, ADC, SBB, AND, SUB, XOR, or CMP");
                    exit(1);
                } break;
            }
        } break;

        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        {
            DecodedInst->DecodeGroup = G8_SHIFT;
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
                    DecodedInst->DecodeGroup = G2_IMM_RM;
                } break;

                case 0x10: // 010
                {
                    DecodedInst->OpcodeEnum = NOT;
                    DecodedInst->DecodeGroup = G3_UNARY_RM;
                } break;

                case 0x18: // 011
                {
                    DecodedInst->OpcodeEnum = NEG;
                    DecodedInst->DecodeGroup = G3_UNARY_RM;
                } break;

                case 0x20: // 100
                {
                    DecodedInst->OpcodeEnum = MUL;
                    DecodedInst->DecodeGroup = G3_UNARY_RM;
                } break;

                case 0x28: // 101
                {
                    DecodedInst->OpcodeEnum = IMUL;
                    DecodedInst->DecodeGroup = G3_UNARY_RM;
                } break;

                case 0x30: // 110
                {
                    DecodedInst->OpcodeEnum = DIV;
                    DecodedInst->DecodeGroup = G3_UNARY_RM;
                } break;

                case 0x38: // 111
                {
                    DecodedInst->OpcodeEnum = IDIV;
                    DecodedInst->DecodeGroup = G3_UNARY_RM;
            } break;

            default:
            { 
                    Debug_OutputErrorMessage("Failed to decode extended opcode 0xF6 or 0xF7:\n   TEST, NOT, NEG, MUL, IMUL, DIV or IDIV"); 
                    exit(1);
                } break;
            }
        } break;

        case 0xFE:
        {
            DecodedInst->DecodeGroup = G3_UNARY_RM;
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
                    DecodedInst->DecodeGroup = G3_UNARY_RM;
                } break;

                case 0x08: // 001
                {
                    DecodedInst->OpcodeEnum = DEC;
                    DecodedInst->DecodeGroup = G3_UNARY_RM;
                } break;

                case 0x10: // 010
                case 0x18: // 011
                {
                    DecodedInst->OpcodeEnum = CALL;
                    DecodedInst->DecodeGroup = G9_MISC;
                } break;

                case 0x20: // 100
                case 0x28: // 101
                {
                    DecodedInst->OpcodeEnum = JMP;
                    DecodedInst->DecodeGroup = G9_MISC;
                } break;

                case 0x30: // 110
                {
                    DecodedInst->OpcodeEnum = PUSH;
                    DecodedInst->DecodeGroup = G3_UNARY_RM;
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

inline u8 *
GetPointerToByteRegister(u16 RegisterToLookUp, union registers *Registers)
{
    u8 *TargetByteRegister = NULLPTR;
    switch(RegisterToLookUp)
    {
        case(AL):
        {
            TargetByteRegister = &Registers->AL;
        } break;

        case(AH):
        {
            TargetByteRegister = &Registers->AH;
        } break;

        case(BL):
        {
            TargetByteRegister = &Registers->BL;
        } break;

        case(BH):
        {
            TargetByteRegister = &Registers->BH;
        } break;

        case(CL):
        {
            TargetByteRegister = &Registers->CL;
        } break;

        case(CH):
        {
            TargetByteRegister = &Registers->CH;
        } break;

        case(DL):
        {
            TargetByteRegister = &Registers->DL;
        } break;

        case(DH):
        {
            TargetByteRegister = &Registers->DH;
        } break;

        default:
        {
            Debug_OutputErrorMessage("Couldn't match byte-width register enum to register struct");
            exit(1);
        }
    }

    if(TargetByteRegister)
    {
        return(TargetByteRegister);
    };

    // below is invalid code path
    Debug_OutputErrorMessage("Couldn't match byte-width register enum to register struct");
    exit(1);
}

inline u16 *
GetPointerToWordRegister(u16 RegisterToLookUp, union registers *Registers)
{
    u16 *TargetWordRegister = NULLPTR;
    switch(RegisterToLookUp)
    {
        case(AX):
        {
            TargetWordRegister = &Registers->AX;
        } break;

        case(BX):
        {
            TargetWordRegister = &Registers->BX;
        } break;

        case(CX):
        {
            TargetWordRegister = &Registers->CX;
        } break;

        case(DX):
        {
            TargetWordRegister = &Registers->DX;
        } break;

        case(SP):
        {
            TargetWordRegister = &Registers->SP;
        } break;

        case(BP):
        {
            TargetWordRegister = &Registers->BP;
        } break;

        case(SI):
        {
            TargetWordRegister = &Registers->SI;
        } break;

        case(DI):
        {
            TargetWordRegister = &Registers->DI;
        } break;

        // todo: enums and registers for IP, flags, segment registers...?

        default:
        {
            Debug_OutputErrorMessage("Couldn't match word-width register enum to register struct");
            exit(1);
        }
    }

    if(TargetWordRegister)
    {
        return(TargetWordRegister);
    };

    // below is invalid code path
    Debug_OutputErrorMessage("Couldn't match word-width register enum to register struct");
    exit(1);
}

void
DoInstruction(struct decoded_inst *DecodedInst, union registers *Registers)
{
    union registers OldRegisters = *Registers;

    switch(DecodedInst->DecodeGroup)
    {
        case 1:
        {
            // [.... ..dw] [mod reg r/m] [disp-lo] [disp-hi]

            // look up reg
            // read r/m
            // if dest flag 
            //      set op1 as reg field and op2 as r/m
            // else
            //      set op2 as reg field and op1 as r/m

            // for now we just handle reg-reg
            
            if(DecodedInst->Mod == REG_MODE)
            {
                if(DecodedInst->IsWord)
                {
                    u16 *DestinationRegister = GetPointerToWordRegister(DecodedInst->OperandOne, Registers);
                    u16 *SourceRegister = GetPointerToWordRegister(DecodedInst->OperandTwo, Registers);
                    *DestinationRegister = *SourceRegister;
                }
                else
                {
                    u8 *DestinationRegister = GetPointerToByteRegister(DecodedInst->OperandOne, Registers);
                    u8 *SourceRegister = GetPointerToByteRegister(DecodedInst->OperandTwo, Registers);
                    *DestinationRegister = *SourceRegister;
                }
                break;
            }
            if(DecodedInst->Mod == MEM_MODE_NO_DISP)
            {

            }

            else if(DecodedInst->Mod == MEM_MODE_DISP_8)
            {

            }

            else if(DecodedInst->Mod == MEM_MODE_DISP_16)
            {

            }

            else
            {
                Debug_OutputErrorMessage("Couldn't route Group 1 instruction based on Mod field");
                exit(1);
            }

            
        } break;

        case 2:
        {
        } break;

        case 3:
        {
        } break;

        case 4:
        {
        } break;

        case 5:
        {
        } break;

        case 6:
        { 
            if(DecodedInst->IsWord)
            {
                u16 *DestinationRegister = GetPointerToWordRegister(DecodedInst->OperandOne, Registers);
                *DestinationRegister = (u16)DecodedInst->OperandTwo;
            }
            else
            {
                u8 *DestinationRegister = GetPointerToByteRegister(DecodedInst->OperandOne, Registers);

                // Safe truncate
                assert(DecodedInst->OperandTwo <= 0xFF);

                u8 TruncatedOperandTwo = (u8)DecodedInst->OperandTwo;
                *DestinationRegister = TruncatedOperandTwo;
            }
        } break;

        case 7:
        {
        } break;

        case 8:
        {
        } break;

        case 9:
        {
        } break;

        default:
        {
            // Error
            Debug_OutputErrorMessage("Couldn't match instruction to DecodeGroup");
            exit(1);
        } break;
    }

    Debug_PrintUpdatedRegisterState(DecodedInst, Registers, &OldRegisters);
}


