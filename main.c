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
    
    return(0);
}
