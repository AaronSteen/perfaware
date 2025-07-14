#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include "sim.h"
#include "functions.c"
#include "dispatch.c"

int 
main(int argc, char **argv)
{
    if(argc != 2)
    {
	Debug_OutputErrorMessage("Error: Usage", __func__, __LINE__);
	exit(1);
    }

    LPCSTR FilePath = argv[1];
    HANDLE FileHandle = Win32_OpenFile(FilePath);
    struct istream IStream = {0};
    
    Win32_LoadInstStream(FileHandle, &IStream);
    u8 *IP = IStream.Start;

    char *Directive = "bits 16\n\n";
    HANDLE HeapHandle = GetProcessHeap();
    struct outbuf OutBuf = {0};
    OutBuf.Start = (char *)HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, 1000);
    OutBuf.Ptr = OutBuf.Start;
    OutBuf.DoNotCrossThisLine = (u8 *)(OutBuf.Ptr + 1000);
    
    // write directive
    SIZE_T DirectiveLen = GetZTermStringLen(Directive);
    WriteToOutBuf(&OutBuf, Directive, DirectiveLen);

    while(IP < IStream.DoNotCrossThisLine) 
    {
	struct decoded_inst DecodedInst = {0};
	DecodedInst.OperandOne.Ptr = DecodedInst.OperandOne.String;
	DecodedInst.OperandTwo.Ptr = DecodedInst.OperandTwo.String;
	DecodedInst.Binary = IP;

	DecodedInst.OpcodeEnum = OpcodeLUT[DecodedInst.Binary[0]];
	if(DecodedInst.OpcodeEnum == EXTENDED)
	{
	    DecodedInst.OpcodeEnum = ReadExtendedOpcode(&DecodedInst);
	}

	if(DecodedInst.OpcodeEnum == MOV)
	{
	    DecodeMOV(&DecodedInst);
	}

	char *InstMnemonic = MnemonicLUT[DecodedInst.OpcodeEnum];
	WriteToOutBuf(&OutBuf, InstMnemonic, GetZTermStringLen(InstMnemonic));
	WriteToOutBuf(&OutBuf, " ", 1);
	WriteToOutBuf(&OutBuf, DecodedInst.OperandOne.String, DecodedInst.OperandOne.Len);
	WriteToOutBuf(&OutBuf, ", ", 2);
	WriteToOutBuf(&OutBuf, DecodedInst.OperandTwo.String, DecodedInst.OperandTwo.Len);
	WriteToOutBuf(&OutBuf, "\n\n", 2);

	IP += DecodedInst.Size;
    }
    SIZE_T OutBufSize = (OutBuf.Ptr - OutBuf.Start);
    Win32_WriteFile((u8 *)OutBuf.Start, OutBufSize);

    return(0);
}

