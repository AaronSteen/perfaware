#include "common.h"

char *EffectiveAddressLUT[] = { "[bx + si", "[bx + di",  "[bp + si", "[bp + di", "[si", "[di", "[bp", "[bx" }; 
char *ByteRegLUT[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
char *WordRegLUT[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
char *SegRegLUT[] = {"es", "cs", "ss", "ds"};
u8 ByteOneToDecodeGroupLUT[256] =
{
    /*00*/ G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G4_ACC_IMM,  G4_ACC_IMM,  G5_OPREG_NODATA, G5_OPREG_NODATA,
    /*08*/ G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G4_ACC_IMM,  G4_ACC_IMM,  G5_OPREG_NODATA, G7_ONEBYTE,      

    /*10*/ G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G4_ACC_IMM,  G4_ACC_IMM,  G5_OPREG_NODATA, G5_OPREG_NODATA,
    /*18*/ G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G4_ACC_IMM,  G4_ACC_IMM,  G5_OPREG_NODATA, G5_OPREG_NODATA,

    /*20*/ G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G4_ACC_IMM,  G4_ACC_IMM,  G7_ONEBYTE,      G7_ONEBYTE,       
    /*28*/ G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G4_ACC_IMM,  G4_ACC_IMM,  G7_ONEBYTE,      G7_ONEBYTE,       

    /*30*/ G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G4_ACC_IMM,  G4_ACC_IMM,  G7_ONEBYTE,      G7_ONEBYTE,       
    /*38*/ G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G4_ACC_IMM,  G4_ACC_IMM,  G7_ONEBYTE,      G7_ONEBYTE,       

    /*40*/ G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA,
    /*48*/ G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA,

    /*50*/ G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA,
    /*58*/ G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA,

    /*60*/ G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,
    /*68*/ G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,

    /*70*/ G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,
    /*78*/ G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,

    /*80*/ G2_IMM_RM,   G2_IMM_RM,   G2_IMM_RM,   G2_IMM_RM,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG, 
    /*88*/ G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G9_MISC,   G1_RM_REG,   G3_UNARY_RM,

    /*90*/ G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA, G5_OPREG_NODATA,
    /*98*/ G7_ONEBYTE,  G7_ONEBYTE,  G9_MISC,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE, 

    /*A0*/ G9_MISC,   G9_MISC,   G9_MISC,   G9_MISC,   G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,   
    /*A8*/ G4_ACC_IMM,  G4_ACC_IMM,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  

    /*B0*/ G6_OPREG_IMM, G6_OPREG_IMM, G6_OPREG_IMM, G6_OPREG_IMM, G6_OPREG_IMM, G6_OPREG_IMM, G6_OPREG_IMM, G6_OPREG_IMM,
    /*B8*/ G6_OPREG_IMM, G6_OPREG_IMM, G6_OPREG_IMM, G6_OPREG_IMM, G6_OPREG_IMM, G6_OPREG_IMM, G6_OPREG_IMM, G6_OPREG_IMM,

    /*C0*/ G7_ONEBYTE,  G7_ONEBYTE,  G9_MISC,  G7_ONEBYTE,  G9_MISC,   G9_MISC,   G2_IMM_RM,   G2_IMM_RM,     
    /*C8*/ G7_ONEBYTE,  G7_ONEBYTE,  G9_MISC,  G7_ONEBYTE,  G7_ONEBYTE,  G9_MISC,  G7_ONEBYTE,    G7_ONEBYTE,       

    /*D0*/ G8_SHIFT,    G8_SHIFT,    G8_SHIFT, G8_SHIFT, G9_MISC, G9_MISC, G7_ONEBYTE, G7_ONEBYTE,
    /*D8*/ G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,   G1_RM_REG,        

    /*E0*/ G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,        
    /*E8*/ G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,  G9_MISC,         

    /*F0*/ G7_ONEBYTE,  G7_ONEBYTE,  G9_MISC,  G9_MISC,  G7_ONEBYTE,  G7_ONEBYTE,  G3_UNARY_RM, G3_UNARY_RM,         
    /*F8*/ G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G7_ONEBYTE,  G3_UNARY_RM, G9_MISC       
};

u8 ByteOneToOpcodeEnumLUT[] =
{
    [0x00] = ADD,       [0x01] = ADD,       [0x02] = ADD,       [0x03] = ADD,
    [0x04] = ADD,       [0x05] = ADD,       [0x06] = PUSH,      [0x07] = POP,

    [0x08] = OR,        [0x09] = OR,        [0x0A] = OR,        [0x0B] = OR,
    [0x0C] = OR,        [0x0D] = OR,        [0x0E] = PUSH,      [0x0F] = NOTUSED,

    [0x10] = ADC,       [0x11] = ADC,       [0x12] = ADC,       [0x13] = ADC,
    [0x14] = ADC,       [0x15] = ADC,       [0x16] = PUSH,      [0x17] = POP,

    [0x18] = SBB,       [0x19] = SBB,       [0x1A] = SBB,       [0x1B] = SBB,
    [0x1C] = SBB,       [0x1D] = SBB,       [0x1E] = PUSH,      [0x1F] = POP,

    [0x20] = AND,       [0x21] = AND,       [0x22] = AND,       [0x23] = AND,
    [0x24] = AND,       [0x25] = AND,       [0x26] = ESprefix,  [0x27] = DAA,

    [0x28] = SUB,       [0x29] = SUB,       [0x2A] = SUB,       [0x2B] = SUB,
    [0x2C] = SUB,       [0x2D] = SUB,       [0x2E] = CSprefix,  [0x2F] = DAS,

    [0x30] = XOR,       [0x31] = XOR,       [0x32] = XOR,       [0x33] = XOR,
    [0x34] = XOR,       [0x35] = XOR,       [0x36] = SSprefix,  [0x37] = AAA,

    [0x38] = CMP,       [0x39] = CMP,       [0x3A] = CMP,       [0x3B] = CMP,
    [0x3C] = CMP,       [0x3D] = CMP,       [0x3E] = DSprefix,  [0x3F] = AAS,

    [0x40] = INC,       [0x41] = INC,       [0x42] = INC,       [0x43] = INC,
    [0x44] = INC,       [0x45] = INC,       [0x46] = INC,       [0x47] = INC,

    [0x48] = DEC,       [0x49] = DEC,       [0x4A] = DEC,       [0x4B] = DEC,
    [0x4C] = DEC,       [0x4D] = DEC,       [0x4E] = DEC,       [0x4F] = DEC,

    [0x50] = PUSH,      [0x51] = PUSH,      [0x52] = PUSH,      [0x53] = PUSH,
    [0x54] = PUSH,      [0x55] = PUSH,      [0x56] = PUSH,      [0x57] = PUSH,

    [0x58] = POP,       [0x59] = POP,       [0x5A] = POP,       [0x5B] = POP,
    [0x5C] = POP,       [0x5D] = POP,       [0x5E] = POP,       [0x5F] = POP,

    [0x60] = NOTUSED,   [0x61] = NOTUSED,   [0x62] = NOTUSED,   [0x63] = NOTUSED,
    [0x64] = NOTUSED,   [0x65] = NOTUSED,   [0x66] = NOTUSED,   [0x67] = NOTUSED,

    [0x68] = NOTUSED,   [0x69] = NOTUSED,   [0x6A] = NOTUSED,   [0x6B] = NOTUSED,
    [0x6C] = NOTUSED,   [0x6D] = NOTUSED,   [0x6E] = NOTUSED,   [0x6F] = NOTUSED,

    [0x70] = JO,        [0x71] = JNO,       [0x72] = JB,    [0x73] = JNB,
    [0x74] = JE,    [0x75] = JNE,   [0x76] = JBE,   [0x77] = JA,

    [0x78] = JS,        [0x79] = JNS,       [0x7A] = JP,    [0x7B] = JNP,
    [0x7C] = JL,    [0x7D] = JNL,   [0x7E] = JLE,   [0x7F] = JG,

    [0x80] = EXTENDED,  [0x81] = EXTENDED,  [0x82] = EXTENDED,  [0x83] = EXTENDED,
    [0x84] = TEST,      [0x85] = TEST,      [0x86] = XCHG,      [0x87] = XCHG,

    [0x88] = MOV,       [0x89] = MOV,       [0x8A] = MOV,       [0x8B] = MOV,
    [0x8C] = MOV,       [0x8D] = LEA,       [0x8E] = MOV,       [0x8F] = POP,

    [0x90] = NOP,       [0x91] = XCHG,      [0x92] = XCHG,      [0x93] = XCHG,
    [0x94] = XCHG,      [0x95] = XCHG,      [0x96] = XCHG,      [0x97] = XCHG,

    [0x98] = CBW,       [0x99] = CWD,       [0x9A] = CALL,      [0x9B] = WAIT,
    [0x9C] = PUSHF,     [0x9D] = POPF,      [0x9E] = SAHF,      [0x9F] = LAHF,

    [0xA0] = MOV,       [0xA1] = MOV,       [0xA2] = MOV,       [0xA3] = MOV,
    [0xA4] = MOVS,      [0xA5] = MOVS,      [0xA6] = CMPS,      [0xA7] = CMPS,

    [0xA8] = TEST,      [0xA9] = TEST,      [0xAA] = STOS,      [0xAB] = STOS,
    [0xAC] = LODS,      [0xAD] = LODS,      [0xAE] = SCAS,      [0xAF] = SCAS,

    [0xB0] = MOV,       [0xB1] = MOV,       [0xB2] = MOV,       [0xB3] = MOV,
    [0xB4] = MOV,       [0xB5] = MOV,       [0xB6] = MOV,       [0xB7] = MOV,

    [0xB8] = MOV,       [0xB9] = MOV,       [0xBA] = MOV,       [0xBB] = MOV,
    [0xBC] = MOV,       [0xBD] = MOV,       [0xBE] = MOV,       [0xBF] = MOV,

    [0xC0] = NOTUSED,   [0xC1] = NOTUSED,   [0xC2] = RET,       [0xC3] = RET,
    [0xC4] = LES,       [0xC5] = LDS,       [0xC6] = MOV,       [0xC7] = MOV,

    [0xC8] = NOTUSED,   [0xC9] = NOTUSED,   [0xCA] = RET,       [0xCB] = RET,
    [0xCC] = INT3,      [0xCD] = INT_,      [0xCE] = INTO,      [0xCF] = IRET,      

    [0xD0] = EXTENDED,  [0xD1] = EXTENDED, [0xD2] = EXTENDED,  [0xD3] = EXTENDED,  
    [0xD4] = AAM,       [0xD5] = AAD,       [0xD6] = NOTUSED,  [0xD7] = XLAT,      

    [0xD8] = ESC,       [0xDF] = ESC,       [0xE0] = LOOPNZ,    [0xE1] = LOOPZ, 
    [0xE2] = LOOP,      [0xE3] = JCXZ,      [0xE8] = CALL,      [0xE9] = JMP,       
    [0xEA] = JMP,       [0xEB] = JMP,       [0xF0] = LOCK,

    [0xF1] = NOTUSED,   [0xF2] = REPNE, [0xF3] = REP,   [0xF4] = HLT,
    [0xF5] = CMC,       [0xF6] = EXTENDED,  [0xF7] = EXTENDED,  [0xF8] = CLC,

    [0xF9] = STC,       [0xFA] = CLI,       [0xFB] = STI,       [0xFC] = CLD,
    [0xFD] = STD,       [0xFE] = EXTENDED,  [0xFF] = EXTENDED,

    [0xE4] = IN_,	[0xEC] = IN_,	    [0xED] = IN_,	[0xE6] = OUT_,
    [0xE7] = OUT_,	[0xEE] = OUT_,	    [0xEF] = OUT_,

}; 

char *OpcodeEnumToStringLUT[255] = {
    [ADD] = "add",        [PUSH] = "push",      [POP] = "pop",        [OR] = "or",
    [ADC] = "adc",        [SBB] = "sbb",        [AND] = "and",        [ESprefix] = "es",

    [DAA] = "daa",        [SUB] = "sub",        [CSprefix] = "cs",    [DAS] = "das",
    [XOR] = "xor",        [SSprefix] = "ss",    [AAA] = "aaa",        [CMP] = "cmp",

    [DSprefix] = "ds",    [AAS] = "aas",        [INC] = "inc",        [DEC] = "dec",
    [JMP] = "jmp",        [JO] = "jo",          [JNO] = "jno",        [JNB] = "jnb",     // jnb / jae / jnc

    [JB] = "jb",     // jb / jnae / jc
    [JE] = "je",     // je / jz
    [JNE] = "jne",    // jne / jnz
    [JBE] = "jbe",    // jbe / jna
    [JA] = "ja",     // jnbe / ja
    [JS] = "js",          [JNS] = "jns",        [JP] = "jp",     // jp / jpe

    [JNP] = "jnp",    // jnp / jpo
    [JL] = "jl",     // jl / jnge
    [JNL] = "jnl",    // jnl / jge
    [JLE] = "jle",    // jle / jng
    [JG] = "jg",     // jnle / jg
    [TEST] = "test",      [XCHG] = "xchg",      [MOV] = "mov",

    [LEA] = "lea",        [NOP] = "nop",        [CBW] = "cbw",        [CWD] = "cwd",
    [CALL] = "call",      [WAIT] = "wait",      [PUSHF] = "pushf",    [POPF] = "popf",

    [SAHF] = "sahf",      [LAHF] = "lahf",      [MOVS] = "movs",      [CMPS] = "cmps",
    [STOS] = "stos",      [LODS] = "lods",      [SCAS] = "scas",      [RET] = "ret",

    [LES] = "les",        [LDS] = "lds",        [INT_] = "int",
    [INTO] = "into",      [IRET] = "iret",
    [ROL] = "rol",        [ROR] = "ror",        [RCL] = "rcl",        [RCR] = "rcr",

    [SHL] = "shl",
    [SHR] = "shr",        [SAR] = "sar",        [AAM] = "aam",
    [AAD] = "aad",        [XLAT] = "xlat",      [ESC] = "esc",        [LOOPNZ] = "loopnz",  // loopne / loopnz

    [LOOPZ] = "loopz",  // loope / loopz
    [LOOP] = "loop",      [JCXZ] = "jcxz",	[IN_] = "in",	      [LOCK] = "lock",
    [REPNE] = "repne",  // repne / repnz
    [REP] = "rep",    // rep / repe / repz
    [HLT] = "hlt",        [CMC] = "cmc",

    [NOT] = "not",        [NEG] = "neg",        [MUL] = "mul",        [IMUL] = "imul",
    [DIV] = "div",        [IDIV] = "idiv",      [CLC] = "clc",        [STC] = "stc",

    [CLI] = "cli",        [STI] = "sti",        [CLD] = "cld",        [STD] = "std",
    [INT3] = "int3",      [EXTENDED] = "extended",                    [NOTUSED] = "notused",

    [OUT_] = "out",
};

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
Debug_PrintCurrentStatus(struct decoded_inst *DecodedInst, int InstStreamIdx)
{
    // Print the instruction we just decoded
    char DecodedInstBuf[MAX_STRING_LEN] = {0};
    sprintf_s(DecodedInstBuf, sizeof(DecodedInstBuf), "\n         Idx %d         \n", InstStreamIdx);
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
    else if( (DecodedInst->OpcodeEnum == LEA) || (DecodedInst->OpcodeEnum == LDS) )
    {
        Group1Decode(DecodedInst);
        char Temp[MAX_STRING_LEN] = {0};
        strncpy(Temp, DecodedInst->OperandOne, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandOne, DecodedInst->OperandTwo, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandTwo, Temp, MAX_STRING_LEN);
    }
    else if(DecodedInst->OpcodeEnum == LES)
    {
        Group1Decode(DecodedInst);
        int RegCode = ( ((DecodedInst->Binary[1]) & FLEX_FIELD) >> 3 );
        strncpy(DecodedInst->OperandTwo, WordRegLUT[RegCode], MAX_STRING_LEN);
        char Temp[MAX_STRING_LEN] = {0};
        strncpy(Temp, DecodedInst->OperandOne, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandOne, DecodedInst->OperandTwo, MAX_STRING_LEN);
        strncpy(DecodedInst->OperandTwo, Temp, MAX_STRING_LEN);
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

    ReadRorMField(&ParsedInst, RorMField);

    strncpy( DecodedInst->OperandOne, RorMField, (MAX_STRING_LEN - strlen(RorMField)) );

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
    ReadImmField(&ParsedInst, ImmBits, ImmField, CareAboutSignExtend, IsSigned);

    strncpy( DecodedInst->OperandOne, RorMField, (MAX_STRING_LEN - strlen(RorMField)) );
    strncpy( DecodedInst->OperandTwo, ImmField, (MAX_STRING_LEN - strlen(ImmField)) );
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

