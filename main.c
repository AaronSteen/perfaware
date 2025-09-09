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

        Dispatch(&DecodedInst);

        Debug_PrintCurrentStatus(&DecodedInst);

        IP += DecodedInst.Size;
        ++IStream.Idx;
    }

    return(0);
}
