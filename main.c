#include "common.h"
#include "source.c"

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
    struct istream IStream = Win32_LoadIStream(FileHandle);
    u8 *IP = IStream.Start;

    while(IP < IStream.DoNotCrossThisLine)
    {
        struct decoded_inst DecodedInst = {0};
        DecodedInst.Binary = IP;

        DecodedInst.OpcodeEnum = ByteOneToOpcodeEnumLUT[DecodedInst.Binary[0]];
        if(DecodedInst.OpcodeEnum == EXTENDED)
        {
            DecodedInst.OpcodeEnum = ReadExtendedOpcode(&DecodedInst);
        }
        DecodedInst.Mnemonic = OpcodeEnumToStringLUT[DecodedInst.OpcodeEnum];

        DecodedInst.DecodeGroup = ByteOneToDecodeGroupLUT[DecodedInst.Binary[0]];
        
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

#if 0
            case 6:
            {
                Group6Decode(&DecodedInst);
            } break;

            case 7:
            {
                Group7Decode(&DecodedInst);
            } break;

            case 8:
            {
                Group8Decode(&DecodedInst);
            } break;

            case 9:
            {
                Group9Decode(&DecodedInst);
            } break;
#endif            
            default:
            {
                // Error
                Debug_OutputErrorMessage("Failed to dispatch in switch statement in main");
                exit(1);
            }


        }

        Debug_PrintCurrentStatus(&DecodedInst, IStream.Idx);

        IP += DecodedInst.Size;
        ++IStream.Idx;
    }

    return(0);
}
