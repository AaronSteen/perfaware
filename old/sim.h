#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <stdint.h>

#define NULLPTR (void *)0
#define NULLCHAR '\0'
#define MAX_STRING_LEN 255
#define REGISTER_NAME_LEN 2

typedef uint8_t u8;
typedef uint16_t u16;
typedef int8_t s8;
typedef int16_t s16;

#define internal static

#define Assert(Expression) if(!Expression) {NULLPTR = 1;}
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

struct string
{
    SIZE_T Len;
    char Start[MAX_STRING_LEN];
    char *Ptr;
    char *DoNotCrossThisLine;
};

struct outbuf
{
    char *Start;
    char *Ptr;
    u8 *DoNotCrossThisLine;
};

struct decoded_inst
{
    u8 *Binary;
    char *ReadableBinary;
    int Size;
    u8 OpcodeEnum;
    char *Mnemonic;
    char *Reg;
    char *RorM;
    struct string OperandOne;
    struct string OperandTwo;
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
    s8 S8;
    u8 U8;
    s16 S16;
    u16 U16;
};

struct int_to_print
{
    enum int_to_print_type Type;
    union int_to_print_data Data;
    struct string String; 
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

// mov
enum
{
    MOV_MEMorREG_TOorFROM_REG = 0x88,
    MOV_IMM_TO_REGorMEM = 0xC6,
    MOV_IMM_TO_REG = 0xB0,
    MOV_MEM_TO_ACCUM = 0xA0,
    MOV_ACCUM_TO_MEM = 0xA2,
    MOV_REGorMEM_TO_SEGREG = 0x8E,
    MOV_SEGREG_TO_REGorMEM = 0x8C,

} MovEnums;

// push
enum
{

    // Push types
				 // [000 reg 110]
        PUSH_SEG_REG = 0x06,     // 0000 0110 (lowest possible value for first byte of segment register push)
				 // 0001 1110 (upper bound of segment register push)
	
				 // [0101 0reg]
	PUSH_REG = 0x50,	 // 0101 0000 (lowest possible value for first byte of register push)
				 // 0101 0111 (upper bound of register push)


				 // [1111 1111] [mod 110 r/m] [disp-lo  ] [disp-hi  ] 
	PUSH_REG_OR_MEM = 0xFF,	 // 1111 1111 (register/memory push always has this as first byte; should if on mod field)
	
    // Segment Registers
	CHECK_SEG_REG = 0x18,    // AND with this and shift right 3 to check the seg reg
				 
} PushEnums;
	
// pop
enum
{

    // Pop types
				 // [000 reg 111]
        POP_SEG_REG = 0x07,     // 0000 0111 (lowest possible value for first byte of segment register pop)
				 // 0001 1111 (upper bound of segment register pop)
	
				 // [0101 1reg]
	POP_REG = 0x58,	 // 0101 1000 (lowest possible value for first byte of register pop)
				 // 0101 1111 (upper bound of register pop)


				 // [1000 1111] [mod 000 r/m] [disp-lo  ] [disp-hi  ] 
	POP_REG_OR_MEM = 0x8F,	 // 1000 1111 (register/memory pop always has this as first byte; should if on mod field)
	
} PopEnums;

// xchg
enum
{
    XCHG_REG_or_MEM_WITH_REG = 0x86,
    XCHG_REG_WITH_ACCUM = 0x90,
    XCHG_UPPER_LIMIT = 0x97
} XchgEnums;

// in
enum
{
    // Fixed port byte one is 0xE4 or 0xE5 if word flag set
    IN_FIXED_PORT = 0xE4, // 1110 010W
    
    // If variable port, instruction byte 1110 110W could be either
    //	  0xEC if Word bit is unset
    //	  0xED if Word bit is set
    IN_VARIABLE_PORT_BYTE = 0xEC, // 1110 1101
    IN_VARIABLE_PORT_WORD = 0xED, // 1110 1101
} InEnums;

enum
{
    // Fixed port byte one is 0xE6 or 0xE7 if word flag set
    OUT_FIXED_PORT = 0xE6, // 1110 011W
    OUT_VARIABLE_PORT_WORD = 0xEF,
} OutEnums;

enum
{

} LoadEnums;

// arithmetic
enum
{
    ARITHMETIC_REGorMEM_WITH_REG_TO_EITHER = 0x00,
    ARITHMETIC_IMM_TO_ACCUM = 0x04,
    ARITHMETIC_IMM_TO_REGorMEM = 0x80,

    ARITH_ADD = 000,
    ARITH_SUB = 101,
    ARITH_CMP = 111

} ArithmeticEnums;

extern char *MnemonicLUT[100];
extern char *ByteRegLUT[];
extern char *WordRegLUT[];
extern char *SegRegLUT[];
extern char *EffectiveAddressLUT[];
extern u8 ByteOneToOpcodeEnumLUT[];
extern char *OpcodeEnumToStringLUT[];
extern u8 OpcodeLUT[];
extern struct outbuf OutBuf;
