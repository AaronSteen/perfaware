#include "sim.h"

void
Debug_OutputErrorMessage(char *ErrorMessage, char *CallingFunction, int Line)
{
    char ErrorBuffer[MAX_STRING_LEN];
    sprintf_s(ErrorBuffer, sizeof(ErrorBuffer), 
	    "\nERROR:\n  In function %s, on line %d,\n\n    ", CallingFunction, Line);
    OutputDebugStringA(ErrorBuffer);
    sprintf_s(ErrorBuffer, sizeof(ErrorBuffer), ErrorMessage);
    OutputDebugStringA(ErrorBuffer);
    OutputDebugStringA(".\n\n");
}

internal void
SteenCopy(struct string *Dest, char *Source, SIZE_T NumBytes)
{
    for(int i = 0; i < NumBytes; i++)
    {
	*Dest->Ptr++ = *Source++;
	Dest->Len++;
    }
}


internal void
DecodeRorMField(struct parsed_inst *ParsedInst, struct decoded_inst *DecodedInst, struct string *RorMString)
{
    struct string DecodedRorM = {0};
    DecodedRorM.Ptr = DecodedRorM.String;

    // init
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    ParsedInst->Binary = DecodedInst->Binary;
    ParsedInst->DestFlag = (bool)((ByteOne & 0x02) >> 1);
    ParsedInst->IsWord = (bool)(ByteOne & 0x01);
    ParsedInst->Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst->Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedInst->RorM = (ByteTwo & R_OR_M_FIELD);


    DecodedInst->Size = 2;

    /* ======= Decode reg field ======= */
    if(ParsedInst->IsWord)
    {
	DecodedInst->Reg = WordRegLUT[ParsedInst->Reg];
    }

    // 8-bit register
    else 
    {
	DecodedInst->Reg = ByteRegLUT[ParsedInst->Reg];
    }

    /* ======= Decode R/M field ======= */
    DecodedRorM.Ptr = DecodedRorM.String;
    
    // Register-to-register mode
    if(ParsedInst->Mod == 0x03)
    {
	if(ParsedInst->IsWord)
	{
	    DecodedInst->RorM = WordRegLUT[ParsedInst->RorM];
	}
	
	// 8-bit register
	else 
	{
	    DecodedInst->RorM = ByteRegLUT[ParsedInst->RorM];
	}
    }

    // Memory mode
    else
    {
	char *StartOfEffAddrReg = EffectiveAddressLUT[ParsedInst->RorM];
	SteenCopy(&DecodedRorM, StartOfEffAddrReg, GetZTermStringLen(StartOfEffAddrReg));

	// No displacement, so just close the bracket and we're done
	if(ParsedInst->Mod == 0x00)
	{
	    SteenCopy(DecodedRorM, "]", 1);
	}
	
	// Otherwise, there's a signed displacement value
	else
	{
	    struct string DispValueBuffer = {0};
	    DispValueBuffer.Ptr = DispValueBuffer.Start;
	    bool IsNegative = false;

	    // Handle 8-bit displacement
	    if(ParsedInst->Mod == 0x01)
	    {
		DecodedInst->Size += 1;

		s8 DispValue = *(s8 *)(ParsedInst->Binary + 2);

		if(DispValue == 0)
		{
		    SteenCopy(DecodedRorM, "]", 1);
		}
		else if(DispValue < 0)

		{
		    IsNegative = true;
		}

		SIZE_T DoNotCross = ( ((char *)DecodedRorM.String + MAX_STRING_LEN) - DecodedRorM.Ptr );
		DecodedRorM.Len = sprintf_s(DecodedRorM.Ptr, DoNotCross, "%d", DispValue);
	    }

	    // Handle 16-bit displacement
	    else if(ParsedInst->Mod == 0x10)
	    {
		DecodedInst->Size += 2;

		s16 DispValue = *(s16 *)(ParsedInst->Binary + 2);

		if(DispValue == 0)
		{
		    SteenCopy(DecodedRorM, "]", 1);
		}

		else if(DispValue < 0)
		{
		    IsNegative = true;
		}

		SIZE_T DoNotCross = ( ((char *)DecodedRorM.String + MAX_STRING_LEN) - DecodedRorM.Ptr );
		DecodedRorM.Len = sprintf_s(DecodedRorM.Ptr, DoNotCross, "%d", DispValue);
	    }

	    // Error
	    else
	    {
		Debug_OutputErrorMessage("Couldn't route instruction to either reg-reg or memory mode\n", __func__, __LINE__);
		exit(1);
	    }

	    // Determine the sign for the displacement arithmetic
	    if(IsNegative)
	    {
		SteenCopy(DecodedRorM, " - ", 3);
		SteenCopy(DecodedRorM, ((char *)DispValueBuffer + 1), (NumDispChars - 1));
	    }
	    else
	    {
		SteenCopy(DecodedRorM, " + ", 3);
		SteenCopy(DecodedRorM, (char *)DispValueBuffer, NumDispChars);
	    }

	    SteenCopy(DecodedRorM, "]", 1);

	    // Special case: Direct address
	    if( (ParsedInst->Mod == 0x00) && (ParsedInst->RorM == 0x06) )
	    {
		DecodedInst.Ptr = DecodedInst.Start;
		DecodedInst.Len = 0;
		SteenCopy(DecodedRorM, "[", 1);
		char DirectAddressBuffer[MAX_STRING_LEN];
		s16 DirectAddress = *(s16 *)(ParsedInst->Binary + 2);
		int NumDirectAddrChars = Get16BitsAsChars(DirectAddressBuffer, DirectAddress);
		SteenCopy(DecodedRorM, DirectAddressBuffer, NumDirectAddrChars);
		SteenCopy(DecodedRorM, "]", 1);
	    }

	    // Write the decoded operands to the struct
	    if(ParsedInst.DestFlag = true)
	    {
		SteenCopy(DecodedInst->OperandOne, DecodedInst->Reg, REGISTER_NAME_LEN);
		SteenCopy(DecodedInst->OperandTwo, DecodedRorM.Start, DecodedRorM.Len);
	    }
	    else
	    {
		SteenCopy(DecodedInst->OperandOne, DecodedRorM.Start, DecodedRorM.Len);
		SteenCopy(DecodedInst->OperandTwo, DecodedInst->Reg, REGISTER_NAME_LEN);
	    }

	}
    }


}




void
OmniDecode(struct parsed_inst *ParsedInst, struct decoded_inst *DecodedInst)
{
    // init
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    ParsedInst->Binary = DecodedInst->Binary;
    ParsedInst->DestFlag = (bool)((ByteOne & 0x02) >> 1);
    ParsedInst->IsWord = (bool)(ByteOne & 0x01);
    ParsedInst->Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst->Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedInst->RorM = (ByteTwo & R_OR_M_FIELD);

    /*struct string DecodedReg = {0};*/
    /*DecodedReg.Ptr = DecodedReg.String;*/
    struct string DecodedRorM = {0};
    DecodedRorM.Ptr = DecodedRorM.String;

    DecodedInst->Size = 2;

    /* ======= Decode reg field ======= */
    if(ParsedInst->IsWord)
    {
	DecodedInst->Reg = WordRegLUT[ParsedInst->Reg];
    }

    // 8-bit register
    else 
    {
	DecodedInst->Reg = ByteRegLUT[ParsedInst->Reg];
    }

    /* ======= Decode R/M field ======= */
    DecodedRorM.Ptr = DecodedRorM.String;
    
    // Register-to-register mode
    if(ParsedInst->Mod == 0x03)
    {
	if(ParsedInst->IsWord)
	{
	    DecodedInst->RorM = WordRegLUT[ParsedInst->RorM];
	}
	
	// 8-bit register
	else 
	{
	    DecodedInst->RorM = ByteRegLUT[ParsedInst->RorM];
	}
    }

    // Memory mode
    else
    {
	char *StartOfEffAddrReg = EffectiveAddressLUT[ParsedInst->RorM];
	SteenCopy(&DecodedRorM, StartOfEffAddrReg, GetZTermStringLen(StartOfEffAddrReg));

	// No displacement, so just close the bracket and we're done
	if(ParsedInst->Mod == 0x00)
	{
	    SteenCopy(DecodedRorM, "]", 1);
	}
	
	// Otherwise, there's a signed displacement value
	else
	{
	    struct string DispValueBuffer = {0};
	    DispValueBuffer.Ptr = DispValueBuffer.Start;
	    bool IsNegative = false;

	    // Handle 8-bit displacement
	    if(ParsedInst->Mod == 0x01)
	    {
		DecodedInst->Size += 1;

		s8 DispValue = *(s8 *)(ParsedInst->Binary + 2);

		if(DispValue == 0)
		{
		    SteenCopy(DecodedRorM, "]", 1);
		}
		else if(DispValue < 0)

		{
		    IsNegative = true;
		}

		SIZE_T DoNotCross = ( ((char *)DecodedRorM.String + MAX_STRING_LEN) - DecodedRorM.Ptr );
		DecodedRorM.Len = sprintf_s(DecodedRorM.Ptr, DoNotCross, "%d", DispValue);
	    }

	    // Handle 16-bit displacement
	    else if(ParsedInst->Mod == 0x10)
	    {
		DecodedInst->Size += 2;

		s16 DispValue = *(s16 *)(ParsedInst->Binary + 2);

		if(DispValue == 0)
		{
		    SteenCopy(DecodedRorM, "]", 1);
		}

		else if(DispValue < 0)
		{
		    IsNegative = true;
		}

		SIZE_T DoNotCross = ( ((char *)DecodedRorM.String + MAX_STRING_LEN) - DecodedRorM.Ptr );
		DecodedRorM.Len = sprintf_s(DecodedRorM.Ptr, DoNotCross, "%d", DispValue);
	    }

	    // Error
	    else
	    {
		Debug_OutputErrorMessage("Couldn't route instruction to either reg-reg or memory mode\n", __func__, __LINE__);
		exit(1);
	    }

	    // Determine the sign for the displacement arithmetic
	    if(IsNegative)
	    {
		SteenCopy(DecodedRorM, " - ", 3);
		SteenCopy(DecodedRorM, ((char *)DispValueBuffer + 1), (NumDispChars - 1));
	    }
	    else
	    {
		SteenCopy(DecodedRorM, " + ", 3);
		SteenCopy(DecodedRorM, (char *)DispValueBuffer, NumDispChars);
	    }

	    SteenCopy(DecodedRorM, "]", 1);

	    // Special case: Direct address
	    if( (ParsedInst->Mod == 0x00) && (ParsedInst->RorM == 0x06) )
	    {
		DecodedInst.Ptr = DecodedInst.Start;
		DecodedInst.Len = 0;
		SteenCopy(DecodedRorM, "[", 1);
		char DirectAddressBuffer[MAX_STRING_LEN];
		s16 DirectAddress = *(s16 *)(ParsedInst->Binary + 2);
		int NumDirectAddrChars = Get16BitsAsChars(DirectAddressBuffer, DirectAddress);
		SteenCopy(DecodedRorM, DirectAddressBuffer, NumDirectAddrChars);
		SteenCopy(DecodedRorM, "]", 1);
	    }

	    // Write the decoded operands to the struct
	    if(ParsedInst.DestFlag = true)
	    {
		SteenCopy(DecodedInst->OperandOne, DecodedInst->Reg, REGISTER_NAME_LEN);
		SteenCopy(DecodedInst->OperandTwo, DecodedRorM.Start, DecodedRorM.Len);
	    }
	    else
	    {
		SteenCopy(DecodedInst->OperandOne, DecodedRorM.Start, DecodedRorM.Len);
		SteenCopy(DecodedInst->OperandTwo, DecodedInst->Reg, REGISTER_NAME_LEN);
	    }

	}
    }
}

internal void
DecodeImmInst(struct parsed_inst *ParsedInst, struct decoded_inst *DecodedInst)
{
    // init
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    ParsedInst->Binary = DecodedInst->Binary;
    ParsedInst->DestFlag = (bool)((ByteOne & 0x02) >> 1);
    ParsedInst->IsWord = (bool)(ByteOne & 0x01);
    ParsedInst->Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst->Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedInst->RorM = (ByteTwo & R_OR_M_FIELD);

    /*struct string DecodedReg = {0};*/
    /*DecodedReg.Ptr = DecodedReg.String;*/
    struct string DecodedRorM = {0};
    DecodedRorM.Ptr = DecodedRorM.String;

    DecodedInst->Size = 3;




    // Imm to Reg/Mem
    // mov [bp + di], byte 7

    /* ======= Decode R/M field ======= */
    char *StartOfEffAddrReg = EffectiveAddressLUT[ParsedInst->RorM];
    SteenCopy(&DecodedRorM, StartOfEffAddrReg, GetZTermStringLen(StartOfEffAddrReg));

    // No displacement, so just close the bracket and we're done
    if(ParsedInst->Mod == 0)
    {
	SteenCopy(&DecodedRorM, "]", 1);
    }

    // Otherwise, there's a signed displacement value
    else
    {
	char DispValueBuffer[MAX_STRING_LEN];
	int NumDispValueChars;
	bool IsNegative = false;
	bool IsZero = false;

	// Handle 8-bit displacement
	if(ParsedInst->Mod == 1)
	{
	    DecodedInst->Size += 1;

	    s8 DispValue = *(s8 *)(ParsedInst->Binary + 2);
	    if(DispValue == 0)
	    {
		IsZero = true;
	    }
	    else if(DispValue < 0)
	    {
		IsNegative = true;
	    }

	    NumDispValueChars = Get8BitsAsChars((char *)DispValueBuffer, DispValue);
	}

	// Handle 16-bit displacement
	else if(ParsedInst->Mod == 2)
	{
	    DecodedInst->Size += 2;

	    s16 DispValue = *(s16 *)(ParsedInst->Binary + 2);
	    if(DispValue == 0)
	    {
		SteenCopy(&DecodedRorM, "]", 1);
	    }

	    else if(DispValue < 0)
	    {
		IsNegative = true;
	    }

	    NumDispValueChars = Get16BitsAsChars((char *)DispValueBuffer, DispValue);
	}

	// Error
	else
	{
	    Debug_OutputErrorMessage("Couldn't route instruction based on mod field", __func__, __LINE__);
	    exit(1);
	}

    }

    







    







    

}
