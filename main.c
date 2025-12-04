#include "common.h"
#include "source.c"
#include "tables.c"

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
    InstStream = Win32_LoadInstStream(FileHandle);
    u8 *IP = InstStream.Start;

    union registers Registers = {0};

    while(IP < InstStream.DoNotCrossThisLine)
    {
        struct decoded_inst DecodedInst = {0};
        DecodedInst.Binary = IP;

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
            default:
            {
                // Error
                Debug_OutputErrorMessage("Failed to dispatch in switch statement in main");
                exit(1);
            } break;
        }

        DoInstruction(&DecodedInst, &Registers);

        Debug_PrintCurrentStatus(&DecodedInst, InstStream.Idx);

        IP += DecodedInst.Size;
        ++InstStream.Idx;
    }

    Debug_PrintFinalRegisterState(&Registers);

    return(0);
}
