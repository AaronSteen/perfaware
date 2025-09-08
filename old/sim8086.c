#include "sim.h"
#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

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


#include "functions.c" 

#define OUTBUF_SIZE 100000 
struct outbuf OutBuf = {0};

int main(int argc, char **argv) 
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
    SIZE_T DirectiveLen = GetZTermStringLen(Directive);
    WriteToOutBuf(Directive, DirectiveLen);

    while(IP < IStream.DoNotCrossThisLine) 
    {
	struct decoded_inst DecodedInst = {0};
	InitString(&DecodedInst.OperandOne);
	InitString(&DecodedInst.OperandTwo);
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
	char InstCounterBuf[MAX_STRING_LEN];
	sprintf_s(InstCounterBuf, sizeof(InstCounterBuf), "Inst %d: ", IStream.Idx);
	// if debug
	WriteToOutBuf(InstCounterBuf, GetZTermStringLen(InstCounterBuf));

	WriteToOutBuf(DecodedInst.Mnemonic, GetZTermStringLen(DecodedInst.Mnemonic));

	if( (DecodedInst.OperandOne.Len > 0) || (DecodedInst.OperandTwo.Len > 0) )
	{
	    WriteToOutBuf(" ", 1);
	    if(DecodedInst.OperandOne.Len > 0)
	    {
		WriteToOutBuf(DecodedInst.OperandOne.String, DecodedInst.OperandOne.Len);
	    }
	    if(DecodedInst.OperandTwo.Len > 0)
	    {
		WriteToOutBuf(", ", 2);
		WriteToOutBuf(DecodedInst.OperandTwo.String, DecodedInst.OperandTwo.Len);
	    }
	}
	WriteToOutBuf("\n\n", 2);
	OutputDebugStringA("\n\n");

	OutputDebugStringA(OutBuf.Start);
	Debug_PrintCurrentStatus(&DecodedInst);

	IStream.Idx++;
	IP += DecodedInst.Size;
    }
    SIZE_T OutBufSize = (OutBuf.Ptr - OutBuf.Start);
    Win32_WriteFile((u8 *)OutBuf.Start, OutBufSize);

    return(0);
}

