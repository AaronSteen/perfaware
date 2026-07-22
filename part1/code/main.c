#define _CRT_SECURE_NO_WARNINGS
#include "common.h"
#include "tables.c"
#include "source.c"

struct inst_stream InstStream = {0};

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
    u8 *Memory = NULL;
    Win32_InitializeMemory(&Memory, &InstStream, FileHandle);

    union registers Registers = {0};

#if 0
    HANDLE HeapHandle = GetProcessHeap();
    u8 *Memory;
    if(HeapHandle)
    {
        Memory = (u8 *)HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, 1024 * 1024);
        assert(Memory != NULL);
    }
#endif

    int Clocks = 0;

    while(InstStream.Start + Registers.IP < InstStream.DoNotCrossThisLine)
    {
        struct decoded_inst DecodedInst = {0};
        DecodedInst.Binary = (u8 *)InstStream.Start + Registers.IP;

        DecodedInst.OpcodeEnum = ByteOneToOpcodeEnumLUT[DecodedInst.Binary[0]];
        if(DecodedInst.OpcodeEnum == EXTENDED)
        {
            ReadExtendedOpcode(&DecodedInst);
        }
        else
        {
            DecodedInst.DecodeGroup = ByteOneToDecodeGroupLUT[DecodedInst.Binary[0]];
        }
        DecodedInst.Mnemonic = OpcodeEnumToStringLUT[DecodedInst.OpcodeEnum];

        switch(DecodedInst.DecodeGroup)
        {
            case 1:
            {
                Group1Decode(&DecodedInst);
            } break;

            case 2:
            {
                Group2Decode(&DecodedInst);
            } break;

            case 3:
            {
                Group3Decode(&DecodedInst);
            } break;

            case 4:
            {
                Group4Decode(&DecodedInst);
            } break;

            case 5:
            {
                Group5Decode(&DecodedInst);
            } break;

            case 6:
            {
                Group6Decode(&DecodedInst);
            } break;

            case 7:
            {
                DecodedInst.Size = 1;
            } break;

            case 8:
            {
                Group8Decode(&DecodedInst);
            } break;

            case 9:
            {
                Group9Decode(&DecodedInst);
            } break;

            case 10:
            {
                Group10Decode(&DecodedInst);
            } break;

            default:
            {
                // Error
                Debug_OutputErrorMessage("Failed to dispatch in switch statement in main");
                exit(1);
            } break;
        }

        union registers OldRegisters = Registers;
        Registers.IP += DecodedInst.Size;
        DoInstruction(&DecodedInst, &Registers, Memory);
        Clocks += DecodedInst.Clocks;

        Debug_PrintUpdatedRegisterState(&DecodedInst, &Registers, &OldRegisters, Clocks);
        // Debug_PrintCurrentStatus(&DecodedInst, InstStream.Idx);

        ++InstStream.Idx;
    }

    Debug_PrintFinalRegisterState(&Registers);
    Win32_WriteImage("image.data", (1024*1024), Memory);

    return(0);
}
