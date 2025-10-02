#pragma once

#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef int8_t s8;
typedef int16_t s16;

#define NULLPTR (void *)0
#define MAX_STRING_LEN 255
#define DIRECT_ADDRESS 0x06
#define Debug_OutputErrorMessage(Msg) __Debug_OutputErrorMessage(Msg, __func__, __LINE__)

struct parsed_inst
{
    bool DestFlag;
    bool IsWord;
    u8 Mod;
    u8 Reg;
    u8 RorM;
    s8 Disp8;
    s16 Disp16;
    u8 AuxiliaryField;
    u8 *Binary;
};

struct istream
{
    LARGE_INTEGER StreamSize;
    u8 *DoNotCrossThisLine;
    u8 *Start;
    int Idx;
};

extern struct istream IStream;
extern char *MnemonicLUT[];
extern char *ByteRegLUT[];
extern char *WordRegLUT[];
extern char *SegRegLUT[];
extern char *EffectiveAddressLUT[];
extern u8 ByteOneToOpcodeEnumLUT[];
extern char *OpcodeEnumToStringLUT[];
extern u8 OpcodeLUT[];

struct decoded_inst
{
    u8 *Binary;
    int Size;
    u8 OpcodeEnum;
    u8 DecodeGroup;
    char *Mnemonic;
    char OperandOne[MAX_STRING_LEN];
    char OperandTwo[MAX_STRING_LEN];
};

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
    JNB = 23,     // jnb / jae / jnc
    JB  = 24,     // jb / jnae / jc
    JE  = 25,     // je / jz
    JNE = 26,     // jne / jnz
    JBE = 27,     // jbe / jna
    JA  = 28,     // jnbe / ja
    JS = 29,
    JNS = 30,
    JP  = 31,     // jp / jpe
    JNP = 32,     // jnp / jpo
    JL  = 33,     // jl / jnge
    JNL = 34,     // jnl / jge
    JLE = 35,     // jle / jng
    JG  = 36,     // jnle / jg
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
    INT_ = 58,      // optionally re-enable
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
    LOOPNZ = 72,   // loopne / loopnz
    LOOPZ  = 73,   // loope / loopz
    LOOP = 74,
    JCXZ = 75,
    IN_ = 76,
    OUT_ = 77,
    LOCK = 78,
    REPNE = 79,    // repne / repnz
    REP   = 80,    // rep / repe / repz
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
    INT3 = 96,
    NOTUSED = 99
} opcodes;

enum
{
    MEM_MODE_NO_DISP = 0x00,
    MEM_MODE_DISP_8 = 0x01,
    MEM_MODE_DISP_16 = 0x02,
    REG_MODE = 0x03,

} Mods;

enum
{
    MOD_FIELD = 0xC0,
    FLEX_FIELD = 0x38,
    R_OR_M_FIELD = 0x07
} Fields;

enum 
{
    G1_RM_REG = 1,   // [.... ..dw] [mod reg r/m] [disp-lo] [disp-hi]
    G2_IMM_RM,       // [.... ..sw] or [.... ..dw] [mod <type> r/m] [disp-lo] [disp-hi] [data] [data]
    G3_UNARY_RM,     // [.... ...w] [mod <type> r/m] [disp-lo] [disp-hi]
    G4_ACC_IMM,      // [.... ...w] [data] [data]
    G5_OPREG_NODATA,        // [.... .reg] (no trailing immediate value)
    G6_OPREG_IMM,    // [.... w reg] [data] [data] (up to two bytes for trailing imm value)
    G7_ONEBYTE,      // one byte with no operands
    G8_SHIFT,        // shifts/rotates D0–D3
    G9_MISC,      // miscellaneous; mostly I/O, control flow
} DecodeGroup;

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

    /*D0*/ G8_SHIFT,    G8_SHIFT,    G8_SHIFT, G8_SHIFT, G7_ONEBYTE, G7_ONEBYTE, G7_ONEBYTE, G7_ONEBYTE,
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






