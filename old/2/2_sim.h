#ifndef SIM_H

#define NULLPTR (void *)0
#define NULLCHAR '\0'
#define MAX_STRING_LEN 255
#define REGISTER_NAME_LEN 2

typedef uint8_t u8;
typedef uint16_t u16;
typedef int8_t s8;
typedef int16_t s16;

#define Assert(Expression) if(!Expression) {NULLPTR = 1;}

// mov
enum
{
    // MOV types
    MEMorREG_TOorFROM_REG = 0x88,
    IMM_TO_REGorMEM = 0xC6,
    IMM_TO_REG = 0xB0,
    MEM_TO_ACCUM = 0xA0,
    ACCUM_TO_MEM = 0xA2,
    REGorMEM_TO_SEGREG = 0x8E,
    SEGREG_TO_REGorMEM = 0x8C,

    // ByteOne fields
    OPCODE = 0xFC,
    D_FLAG = 0x02,
    W_FLAG = 0x01,

    // ByteTwo fields
    MOD_FIELD = 0xC0, // 1100 0000
    FLEX_FIELD = 0x38, // 0011 1000, very flexible field
    R_OR_M_FIELD = 0x07, // 0000 0111

    // MOD field
    MEM_MODE_NO_DISP = 0x00,
    MEM_MODE_DISP8 = 0x01,
    MEM_MODE_DISP16 = 0x02,
    REG_MODE = 0x03,

    // Special case
    DIRECT_ADDRESS = 0x06
} MovEnums;

struct parsed_mov
{
    bool DestFlag;
    bool IsWord;
    u8 Mod;
    u8 Reg;
    u8 RorM;
    u8 *Binary;
};

struct istream
{
    LARGE_INTEGER StreamSize;
    u8 *DoNotCrossThisLine;
    u8 *Start;
};

struct string
{
    SIZE_T Len;
    char String[MAX_STRING_LEN];
    char *Ptr;
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
    int Size;
    u8 OpcodeEnum;
    struct string OperandOne;
    struct string OperandTwo;
};

char *MnemonicLUT[100] = {[39] = "mov"};

char *ByteRegLUT[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
char *WordRegLUT[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};

char *EffectiveAddressLUT[] =
    { "[bx + si", "[bx + di",  "[bp + si", "[bp + di", "[si", "[di", "[bp", "[bx" };

u8 *Win32_ReadFile(HANDLE FileHandle);
HANDLE Win32_OpenFile(LPCSTR FilePath);
void Debug_OutputErrorMessage(char *ErrorMessage, char *CallingFunction, int Line);
void SteenCopy(struct string *Dest, char *Source, SIZE_T NumBytes);
SIZE_T GetZTermStringLen(char *String);
void Win32_WriteFile(u8 *OutBuffer, SIZE_T OutBufferSize);
void Win32_LoadInstStream(HANDLE FileHandle, struct istream *IStream);
int DecodeMnemonic(u8 FirstByte);
char *GetEffectiveAddress(u8 RorMField);
int Get8BitsAsChars(char *PrintBuffer, s8 Bits);
int Get16BitsAsChars(char *PrintBuffer, s16 Bits);
u8 ReadExtendedOpcode(struct decoded_inst *DecodedInst);
void DecodeMOV(struct decoded_inst *DecodedInst);
:wa
char *DecodeReg(bool WordFlag, u8 RegField);
void DecodeRorM(struct parsed_mov *ParsedMov, struct string *DecodedRorM);
void WriteToOutBuf(struct outbuf *OutBuf, char *Source, SIZE_T NumBytes);

#define SIM_H
#endif
