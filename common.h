#pragma once

#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;

#define NULLPTR (void *)0
#define MAX_STRING_LEN 255
#define DIRECT_ADDRESS 0x06
#define Debug_OutputErrorMessage(Msg) __Debug_OutputErrorMessage(Msg, __func__, __LINE__)

struct inst_stream
{
    LARGE_INTEGER StreamSize;
    u8 *DoNotCrossThisLine;
    u8 *Start;
    int Idx;
};

extern struct inst_stream InstStream;
extern char *MnemonicLUT[];
extern char *ByteRegLUT[];
extern char *WordRegLUT[];
extern char *OtherRegLUT[];
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
    bool DestFlag;
    bool IsWord;
    u8 Mod;
    u8 Reg;
    u8 RorM;
    char OperandOneStr[MAX_STRING_LEN];
    char OperandTwoStr[MAX_STRING_LEN];
    u16 OperandOne;
    u16 OperandTwo;
};

#define NUMBER_OF_WORD_SIZED_REGISTERS 14
union registers
{
    struct
    {
        union 
        {
            u16 AX; struct { u8 AL, AH; };
        };

        union 
        {
            u16 BX; struct { u8 BL, BH; };
        };

        union 
        {
            u16 CX; struct { u8 CL, CH; };
        };

        union 
        {
            u16 DX; struct { u8 DL, DH; };   
        };

        u16 SP, BP, SI, DI, ES, CS, SS, DS, IP, Flags;
    };

    u16 WordRegisters[NUMBER_OF_WORD_SIZED_REGISTERS];
    u8 ByteRegisters[NUMBER_OF_WORD_SIZED_REGISTERS][2];

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
};

enum
{
    MEM_MODE_NO_DISP = 0x00,
    MEM_MODE_DISP_8 = 0x01,
    MEM_MODE_DISP_16 = 0x02,
    REG_MODE = 0x03,

};

enum
{
    MOD_FIELD = 0xC0,
    FLEX_FIELD = 0x38,
    R_OR_M_FIELD = 0x07
};










