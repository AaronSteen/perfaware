#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define NULLPTR (void *)0
#define NULLCHAR '\0'
#define MAX_STRING_LEN 255
#define REGISTER_NAME_LEN 2

typedef uint8_t u8;
typedef uint16_t u16;
typedef int8_t s8;
typedef int16_t s16;

struct decoded_inst
{
    u8 *Binary;
    char *ReadableBinary;
    int Size;
    u8 OpcodeEnum;
    char *Mnemonic;
    char *Reg;
    char *RorM;
    char OperandOne[MAX_STRING_LEN];
    char OperandTwo[MAX_STRING_LEN];
};

struct istream
{
    LARGE_INTEGER StreamSize;
    u8 *DoNotCrossThisLine;
    u8 *Start;
    int Idx;
};

struct outbuf
{
    char *Start;
    char *Ptr;
    u8 *DoNotCrossThisLine;
};

struct parsed_inst
{
    BOOL DestFlag;
    BOOL IsWord;
    u8 Mod;
    u8 Reg;
    u8 RorM;
    s8 Disp8;
    s16 Disp16;
    u8 AuxiliaryField;
    u8 *Binary;
};

enum int_to_print_type
{
    SIGNED_8 = 0,
    UNSIGNED_8 = 1,
    SIGNED_16 = 2,
    UNSIGNED_16 = 3,
};

union int_to_print_data
{
    s8 Sint8;
    u8 Uint8;
    s16 Sint16;
    u16 Uint16;
};

struct int_to_print
{
    enum int_to_print_type Type;
    union int_to_print_data Data;
};


#define OUTBUF_SIZE 100000 
struct outbuf OutBuf = {0};

enum
{
    // ByteTwo fields
    MOD_FIELD = 0xC0, // 1100 0000
    FLEX_FIELD = 0x38, // 0011 1000, very flexible field
    R_OR_M_FIELD = 0x07, // 0000 0111

    // MOD field
    MEM_MODE_NO_DISP = 0x00,
    MEM_MODE_DISP8 = 0x01,
    MEM_MODE_DISP16 = 0x02,
    REG_MODE = 0x03,

    DIRECT_ADDRESS = 0x06

} CommonEnums;

enum
{
    ADD = 0,
    PUSH = 1,
    POP = 2,
    OR = 3,
    ADC = 4,
    SBB = 5,
    AND = 6,
    ESprefix = 7,
    DAA = 8,
    SUB = 9,
    CSprefix = 10,
    DAS = 11,
    XOR = 12,
    SSprefix = 13,
    AAA = 14,
    CMP = 15,
    DSprefix = 16,
    AAS = 17,
    INC = 18,
    DEC = 19,
    JMP = 20,
    JO = 21,
    JNO = 22,
    TBD_JNB = 23,     // jnb / jae / jnc
    TBD_JB  = 24,     // jb / jnae / jc
    TBD_JE  = 25,     // je / jz
    TBD_JNE = 26,     // jne / jnz
    TBD_JBE = 27,     // jbe / jna
    TBD_JA  = 28,     // jnbe / ja
    JS = 29,
    JNS = 30,
    TBD_JP  = 31,     // jp / jpe
    TBD_JNP = 32,     // jnp / jpo
    TBD_JL  = 33,     // jl / jnge
    TBD_JGE = 34,     // jnl / jge
    TBD_JLE = 35,     // jle / jng
    TBD_JG  = 36,     // jnle / jg
    TEST = 37,
    XCHG = 38,
    MOV = 39,
    LEA = 40,
    NOP = 41,
    CBW = 42,
    CWD = 43,
    CALL = 44,
    WAIT = 45,
    PUSHF = 46,
    POPF = 47,
    SAHF = 48,
    LAHF = 49,
    MOVS = 50,
    CMPS = 51,
    STOS = 52,
    LODS = 53,
    SCAS = 54,
    RET = 55,
    LES = 56,
    LDS = 57,
    /*INT = 58,*/      // optionally re-enable
    INTO = 59,
    IRET = 60,
    ROL = 61,
    ROR = 62,
    RCL = 63,
    RCR = 64,
    SHL = 65,      // shl / sal
    SHR = 66,
    SAR = 67,
    AAM = 68,
    AAD = 69,
    XLAT = 70,
    ESC = 71,
    TBD_LOOPNE = 72,   // loopne / loopnz
    TBD_LOOPE  = 73,   // loope / loopz
    LOOP = 74,
    JCXZ = 75,
    IN_ = 76,
    OUT_ = 77,
    LOCK = 78,
    TBD_REPNE = 79,    // repne / repnz
    TBD_REP   = 80,    // rep / repe / repz
    HLT = 81,
    CMC = 82,
    NOT = 83,
    NEG = 84,
    MUL = 85,
    IMUL = 86,
    DIV = 87,
    IDIV = 88,
    CLC = 89,
    STC = 90,
    CLI = 91,
    STI = 92,
    CLD = 93,
    STD = 94,
    EXTENDED = 95,
    NOTUSED = 99
} opcodes;

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

    [0x70] = JO,        [0x71] = JNO,       [0x72] = TBD_JB,    [0x73] = TBD_JNB,
    [0x74] = TBD_JE,    [0x75] = TBD_JNE,   [0x76] = TBD_JBE,   [0x77] = TBD_JA,

    [0x78] = JS,        [0x79] = JNS,       [0x7A] = TBD_JP,    [0x7B] = TBD_JNP,
    [0x7C] = TBD_JL,    [0x7D] = TBD_JGE,   [0x7E] = TBD_JLE,   [0x7F] = TBD_JG,

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
    [0xCE] = INTO,      [0xCF] = IRET,      [0xD0] = EXTENDED,  [0xD1] = EXTENDED,

    [0xD2] = EXTENDED,  [0xD3] = EXTENDED,  [0xD4] = AAM,       [0xD5] = AAD,       
    [0xD6] = NOTUSED,
    [0xD7] = XLAT,      [0xD8] = ESC,       [0xDF] = ESC,       [0xE0] = TBD_LOOPNE,

    [0xE1] = TBD_LOOPE, [0xE2] = LOOP,      [0xE3] = JCXZ,      [0xE8] = CALL,
    [0xE9] = JMP,       [0xEA] = JMP,       [0xEB] = JMP,       [0xF0] = LOCK,

    [0xF1] = NOTUSED,   [0xF2] = TBD_REPNE, [0xF3] = TBD_REP,   [0xF4] = HLT,
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
    [JMP] = "jmp",        [JO] = "jo",          [JNO] = "jno",        [TBD_JNB] = "TBD",     // jnb / jae / jnc

    [TBD_JB] = "TBD",     // jb / jnae / jc
    [TBD_JE] = "TBD",     // je / jz
    [TBD_JNE] = "TBD",    // jne / jnz
    [TBD_JBE] = "TBD",    // jbe / jna
    [TBD_JA] = "TBD",     // jnbe / ja
    [JS] = "js",          [JNS] = "jns",        [TBD_JP] = "TBD",     // jp / jpe

    [TBD_JNP] = "TBD",    // jnp / jpo
    [TBD_JL] = "TBD",     // jl / jnge
    [TBD_JGE] = "TBD",    // jnl / jge
    [TBD_JLE] = "TBD",    // jle / jng
    [TBD_JG] = "TBD",     // jnle / jg
    [TEST] = "test",      [XCHG] = "xchg",      [MOV] = "mov",

    [LEA] = "lea",        [NOP] = "nop",        [CBW] = "cbw",        [CWD] = "cwd",
    [CALL] = "call",      [WAIT] = "wait",      [PUSHF] = "pushf",    [POPF] = "popf",

    [SAHF] = "sahf",      [LAHF] = "lahf",      [MOVS] = "movs",      [CMPS] = "cmps",
    [STOS] = "stos",      [LODS] = "lods",      [SCAS] = "scas",      [RET] = "ret",

    [LES] = "les",        [LDS] = "lds",        [INTO] = "into",      [IRET] = "iret",
    [ROL] = "rol",        [ROR] = "ror",        [RCL] = "rcl",        [RCR] = "rcr",

    [SHL] = "shl",
    [SHR] = "shr",        [SAR] = "sar",        [AAM] = "aam",
    [AAD] = "aad",        [XLAT] = "xlat",      [ESC] = "esc",        [TBD_LOOPNE] = "TBD",  // loopne / loopnz

    [TBD_LOOPE] = "TBD",  // loope / loopz
    [LOOP] = "loop",      [JCXZ] = "jcxz",	[IN_] = "in",	      [LOCK] = "lock",
    [TBD_REPNE] = "TBD",  // repne / repnz
    [TBD_REP] = "TBD",    // rep / repe / repz
    [HLT] = "hlt",        [CMC] = "cmc",

    [NOT] = "not",        [NEG] = "neg",        [MUL] = "mul",        [IMUL] = "imul",
    [DIV] = "div",        [IDIV] = "idiv",      [CLC] = "clc",        [STC] = "stc",

    [CLI] = "cli",        [STI] = "sti",        [CLD] = "cld",        [STD] = "std",
    [EXTENDED] = "extended", [NOTUSED] = "notused",

    [OUT_] = "out",
};

char *EffectiveAddressLUT[] = { "[bx + si", "[bx + di",  "[bp + si", "[bp + di", "[si", "[di", "[bp", "[bx" }; 
char *ByteRegLUT[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
char *WordRegLUT[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
char *SegRegLUT[] = {"es", "cs", "ss", "ds"};

void
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

#define Debug_OutputErrorMessage(Msg) __Debug_OutputErrorMessage(Msg, __func__, __LINE__)


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
                    BOOL Result = ReadFile(FileHandle, IStream->Start, 
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

void
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

char *
DecodeReg(BOOL WordFlag, u8 RegCode)
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
GetIntAsChars(char *IntToPrintString, struct int_to_print IntToPrint)
{
    int SpaceLeft = MAX_STRING_LEN - strlen(IntToPrintString);

    switch(IntToPrint.Type)
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



void
DecodeRorM(struct parsed_inst *ParsedInst, char *DecodedRorM, struct decoded_inst *DecodedInst)
{
    if(ParsedInst->Mod == REG_MODE)
    {
        char *WhichRegIsIt = DecodeReg(ParsedInst->IsWord, ParsedInst->RorM);
        strncat(DecodedRorM, WhichRegIsIt, REGISTER_NAME_LEN);
        return;
    }
    else
    {
        if( (ParsedInst->Mod == MEM_MODE_NO_DISP) && (ParsedInst->RorM == DIRECT_ADDRESS) )
        {
            DecodedInst->Size = 4;
            // special case
            strncat(DecodedRorM, "[", 1);
            char DirectAddressBuffer[MAX_STRING_LEN];
            s16 DirectAddress = *(s16 *)(ParsedInst->Binary + 2);
            int NumDirectAddrChars = Get16BitsAsChars(DirectAddressBuffer, DirectAddress);
            strncat(DecodedRorM, DirectAddressBuffer, NumDirectAddrChars);
            strncat(DecodedRorM, "]", 1);
            return;
        }
        char *DestAddr = GetEffectiveAddress(ParsedInst->RorM);
        SIZE_T DestAddrLen = strlen(DestAddr);
        strncat(DecodedRorM, DestAddr, DestAddrLen);

        if(ParsedInst->Mod == MEM_MODE_NO_DISP)
        {
            // If no displacement just close the bracket
            strncat(DecodedRorM, "]", 1);
            return;
        }
        else
        {
            char DispValueBuffer[MAX_STRING_LEN];
            int NumDispChars = 0;
            BOOL IsNegative = false;

            if(ParsedInst->Mod == MEM_MODE_DISP8)
            {
                s8 DispValue = *(s8 *)(ParsedInst->Binary + 2);
                if(DispValue == 0)
                {
                    strncat(DecodedRorM, "]", 1);
                    return;
                }
                else if(DispValue < 0)
                {
                    IsNegative = true;
                }
                NumDispChars = Get8BitsAsChars(DispValueBuffer, DispValue);
            }
            else if(ParsedInst->Mod == MEM_MODE_DISP16)
            {
                s16 DispValue = *(s16 *)(ParsedInst->Binary + 2);
                if(DispValue == 0)
                {
                    strncat(DecodedRorM, "]", 1);
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
                strncat(DecodedRorM, " - ", 3);
                strncat(DecodedRorM, ((char *)DispValueBuffer + 1), (NumDispChars - 1));
            }
            else
            {
                strncat(DecodedRorM, " + ", 3);
                strncat(DecodedRorM, (char *)DispValueBuffer, NumDispChars);
            }

            strncat(DecodedRorM, "]", 1);
        }
    }

}

// r/m to or from reg; second byte is [mod reg r/m]; up to two displacement bytes
//      e.g., MOV, ADD, XOR
void
Group1Decode(struct decoded_inst *DecodedInst)
{
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];
    struct parsed_inst ParsedInst = {0};
    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.DestFlag = (BOOL)((ByteOne & 0x02) >> 1);
    ParsedInst.IsWord = (BOOL)(ByteOne & 0x01);
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    char DecodedReg[MAX_STRING_LEN];
    char DecodedRorM[MAX_STRING_LEN];

	char *WhichRegIsIt = DecodeReg(ParsedMov.IsWord, ParsedMov.Reg);
    strncpy(DecodedReg, WhichRegIsIt, REGISTER_NAME_LEN);

	DecodeRorM(&ParsedMov, &DecodedRorM, DecodedInst);
    




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
                DecodeMov(DecodedInst);
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

int
main(int argc, char **argv)
{
    if(argc != 2) 
    { 
        Debug_OutputErrorMessage("Error: Usage"); 
        exit(1); 
    } 

    LPCSTR FilePath = argv[1]; 
    HANDLE FileHandle = Win32_OpenFile(FilePath); 
    struct istream IStream = {0}; 
    Win32_LoadInstStream(FileHandle, &IStream); 
    u8 *IP = IStream.Start;

    char *Directive = "bits 16\n\n";
    HANDLE HeapHandle = GetProcessHeap();
    OutBuf.Start = (char *)HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, OUTBUF_SIZE);
    OutBuf.Ptr = OutBuf.Start;
    OutBuf.DoNotCrossThisLine = (u8 *)(OutBuf.Ptr + OUTBUF_SIZE);

    // write directive
    SIZE_T DirectiveLen = strlen(Directive);
    WriteToOutBuf(Directive, DirectiveLen);

    while(IP < IStream.DoNotCrossThisLine) 
    {
        struct decoded_inst DecodedInst = {0};
        DecodedInst.Binary = IP;

        // if(IStream.Idx == 166)
        // {
        //     __debugbreak();
        // }
        //
        DecodedInst.OpcodeEnum = ByteOneToOpcodeEnumLUT[DecodedInst.Binary[0]];
        if(DecodedInst.OpcodeEnum == EXTENDED)
        {
            DecodedInst.OpcodeEnum = ReadExtendedOpcode(&DecodedInst);
        }
        DecodedInst.Mnemonic = OpcodeEnumToStringLUT[DecodedInst.OpcodeEnum];


        Dispatch(&DecodedInst);

        if(SteenCmp(DecodedInst.Mnemonic, "TBD", 3))
        {
            Debug_OutputErrorMessage("TBD Opcode");
            exit(1);
        }
        // char InstCounterBuf[MAX_STRING_LEN];
        // sprintf_s(InstCounterBuf, sizeof(InstCounterBuf), "Inst %d: ", IStream.Idx);
        // // if debug
        // WriteToOutBuf(InstCounterBuf, strlen(InstCounterBuf));
        //
        // WriteToOutBuf(DecodedInst.Mnemonic, strlen(DecodedInst.Mnemonic));
        //
        // if( (DecodedInst.OperandOne.Len > 0) || (DecodedInst.OperandTwo.Len > 0) )
        // {
        //     WriteToOutBuf(" ", 1);
        //     if(DecodedInst.OperandOne.Len > 0)
        //     {
        //         WriteToOutBuf(DecodedInst.OperandOne.String, DecodedInst.OperandOne.Len);
        //     }
        //     if(DecodedInst.OperandTwo.Len > 0)
        //     {
        //         WriteToOutBuf(", ", 2);
        //         WriteToOutBuf(DecodedInst.OperandTwo.String, DecodedInst.OperandTwo.Len);
        //     }
        // }
        // WriteToOutBuf("\n\n", 2);
        // OutputDebugStringA("\n\n");
        //
        // OutputDebugStringA(OutBuf.Start);
        Debug_PrintCurrentStatus(&DecodedInst);

        IStream.Idx++;
        IP += DecodedInst.Size;
    }
    // SIZE_T OutBufSize = (OutBuf.Ptr - OutBuf.Start);
    // Win32_WriteFile((u8 *)OutBuf.Start, OutBufSize);

    return(0);}
