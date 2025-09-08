#include "sim.h"

internal void
Debug_PrintBinary(u8 Value)
{
    for(int i = 0; i < 8; i++)
    {
	int ShiftAmt = (7 - i);
	u8 Digit = ((Value >> ShiftAmt) & 1);
	if(Digit)
	{
	    OutputDebugStringA("1");
	}
	else
	{
	    OutputDebugStringA("0");
	}
	if(i == 3)
	{
	    OutputDebugStringA(" ");
	}
    }
}

internal void
__Debug_OutputErrorMessage(char *ErrorMessage, char *CallingFunction, int Line)
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
Debug_PrintCurrentStatus(struct decoded_inst *DecodedInst)
{
    /*char ErrorBuffer[MAX_STRING_LEN];*/
    /**/
    /*OutputDebugStringA("\n\n");*/
    /**/
    /*OutputDebugStringA("Just decoded inst, Hex: ");*/
    /*sprintf_s(ErrorBuffer, sizeof(ErrorBuffer), "0x%02X", DecodedInst->Binary[0]);*/
    /*OutputDebugStringA(ErrorBuffer);*/
    /*OutputDebugStringA("\n\n");*/

    for(int i = 0; i < 10; i++)
    {
	char NumByteToPrintBuf[MAX_STRING_LEN];
	int NumByteToPrint;
	if(i < DecodedInst->Size)
	{
	    NumByteToPrint = i + 1;
	}
	else
	{
	    NumByteToPrint = ((i - DecodedInst->Size) + 1);
	}

	sprintf_s(NumByteToPrintBuf, sizeof(NumByteToPrintBuf), "Inst byte %d:", NumByteToPrint);
	OutputDebugStringA(NumByteToPrintBuf);
	u8 InstByte = *((DecodedInst->Binary) + i);
	Debug_PrintBinary(InstByte);
	OutputDebugStringA("\n\n");
	if(i == (DecodedInst->Size - 1))
	{
	    OutputDebugStringA("\n============================\n");

	    u8 NextInstByteOne = *(DecodedInst->Binary + DecodedInst->Size);
	    u8 NextInstOpcodeEnum = ByteOneToOpcodeEnumLUT[NextInstByteOne];
	    char *NextInstOpcodeString;
	    if(NextInstOpcodeEnum == EXTENDED)
	    {
		NextInstOpcodeString = "Extended";
	    }
	    else
	    {
		NextInstOpcodeString = OpcodeEnumToStringLUT[NextInstOpcodeEnum];
	    }
	    OutputDebugStringA("\n    Next inst: ");
	    OutputDebugStringA(NextInstOpcodeString);
	}
	OutputDebugStringA("\n\n");
    }
    OutputDebugStringA("\n\n");

}

internal void
SteenCopy(struct string *Dest, char *Src, int BytesToCopy)
{
    if( (Dest->Ptr + BytesToCopy) <= Dest->DoNotCrossThisLine )
    {
        for(int i = 0; i < BytesToCopy; ++i)
        {
            *(Dest->Ptr++) = *Src++;
        }
        *Dest->Ptr = NULLCHAR;
    }
    else
    {
        Debug_OutputErrorMessage("Buffer overflow");
        exit(1);

    }
}

internal char *
DecodeReg(bool WordFlag, u8 RegCode)
{
    if(WordFlag)
    {
	return(WordRegLUT[RegCode]);
    }
    else
    {
	return(ByteRegLUT[RegCode]);    
    }
}

internal void
DecodeRorM(struct parsed_inst *ParsedInst, struct string *DecodedRorM, struct decoded_inst *DecodedInst)
{
    if(ParsedInst->Mod == REG_MODE)
    {
	char *WhichRegIsIt = DecodeReg(ParsedInst->IsWord, ParsedInst->RorM);
	SteenCopy(DecodedRorM, WhichRegIsIt, REGISTER_NAME_LEN);
	return;
    }
    else
    {
	if( (ParsedInst->Mod == MEM_MODE_NO_DISP) && (ParsedInst->RorM == DIRECT_ADDRESS) )
	{
	    DecodedInst->Size = 4;
	    // special case
	    SteenCopy(DecodedRorM, "[", 1);
	    char DirectAddressBuffer[MAX_STRING_LEN];
	    s16 DirectAddress = *(s16 *)(ParsedInst->Binary + 2);
	    int NumDirectAddrChars = Get16BitsAsChars(DirectAddressBuffer, DirectAddress);
	    SteenCopy(DecodedRorM, DirectAddressBuffer, NumDirectAddrChars);
	    SteenCopy(DecodedRorM, "]", 1);
	    return;
	}
	char *DestAddr = GetEffectiveAddress(ParsedInst->RorM);
	SIZE_T DestAddrLen = GetZTermStringLen(DestAddr);
	SteenCopy(DecodedRorM, DestAddr, DestAddrLen);

	if(ParsedInst->Mod == MEM_MODE_NO_DISP)
	{
	    // If no displacement just close the bracket
	    SteenCopy(DecodedRorM, "]", 1);
	    return;
	}
	else
	{
	    char DispValueBuffer[MAX_STRING_LEN];
	    int NumDispChars = 0;
	    bool IsNegative = false;

	    if(ParsedInst->Mod == MEM_MODE_DISP8)
	    {
		s8 DispValue = *(s8 *)(ParsedInst->Binary + 2);
		if(DispValue == 0)
		{
		    SteenCopy(DecodedRorM, "]", 1);
		    return;
		}
		else if(DispValue < 0)
		{
		    IsNegative = true;
		}
		NumDispChars = Get8BitsAsChars(DispValueBuffer, DispValue);
	    }
	    else if(ParsedInst->Mod == MEM_MODE_DISP16)
	    {
		s16 DispValue = *(s16 *)(ParsedInst->Binary + 2);
		if(DispValue == 0)
		{
		    SteenCopy(DecodedRorM, "]", 1);
		    return;
		}
		else if(DispValue < 0)
		{
		    IsNegative = true;
		}
		NumDispChars = Get16BitsAsChars(DispValueBuffer, DispValue);
	    }
	    else
	    {
		// Error
		exit(1);
	    }

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
	}
    }

}



internal void
OmniDecode(struct decoded_inst *DecodedInst, struct parsed_inst *ParsedInst,
        struct string *DecodedReg, struct string *DecodedRorM)
{
    DecodedInst->Size = 2;

    char *WhichRegIsIt = DecodeReg(ParsedInst->IsWord, ParsedInst->Reg);
    SteenCopy(DecodedReg, WhichRegIsIt, 2);

    DecodeRorM(ParsedInst, DecodedRorM, DecodedInst);
    if(ParsedInst->Mod == MEM_MODE_DISP8)
    {
        DecodedInst->Size = 3;
    }
    else if( (ParsedInst->Mod == MEM_MODE_DISP16) ||
            (ParsedInst->Mod == MEM_MODE_NO_DISP && ParsedInst->RorM == DIRECT_ADDRESS) )
    {
        DecodedInst->Size = 4;
    }
}

internal void
DecodeMov(struct decoded_inst *DecodedInst)
{
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];
    struct parsed_inst ParsedInst = {0};
    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.DestFlag = (bool)((ByteOne & 0x02) >> 1);
    ParsedInst.IsWord = (bool)(ByteOne & 0x01);
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    struct string DecodedReg = {0};
    InitString(&DecodedReg);
    struct string DecodedRorM = {0};
    InitString(&DecodedRorM);

    if( (ByteOne >= MOV_MEMorREG_TOorFROM_REG) && (ByteOne < MOV_SEGREG_TO_REGorMEM) )
    {
        DecodedInst->Size = 2;

        char *WhichRegIsIt = DecodeReg(ParsedInst.IsWord, ParsedInst.Reg);
        SteenCopy(&DecodedReg, WhichRegIsIt, REGISTER_NAME_LEN);

        DecodeRorM(&ParsedInst, &DecodedRorM, DecodedInst);
        if(ParsedInst.Mod == MEM_MODE_DISP8)
        {
            DecodedInst->Size = 3;
        }
        else if( (ParsedInst.Mod == MEM_MODE_DISP16) ||
                (ParsedInst.Mod == MEM_MODE_NO_DISP && ParsedInst.RorM == DIRECT_ADDRESS) )
        {
            DecodedInst->Size = 4;
        }
        if(ParsedInst.DestFlag)
        {
            SteenCopy(&DecodedInst->OperandOne, DecodedReg.String, DecodedReg.Len);
            SteenCopy(&DecodedInst->OperandTwo, DecodedRorM.String, DecodedRorM.Len);
        }
        else
        {
            SteenCopy(&DecodedInst->OperandOne, DecodedRorM.String, DecodedRorM.Len);
            SteenCopy(&DecodedInst->OperandTwo, DecodedReg.String, DecodedReg.Len);
        }
    }

    else if( (ByteOne >= MOV_SEGREG_TO_REGorMEM) && (ByteOne < MOV_REGorMEM_TO_SEGREG) )
    {
        Debug_OutputErrorMessage("MOV_SEGREG_TO_REGorMEM");
        // Not yet
    }

    else if( (ByteOne >= MOV_REGorMEM_TO_SEGREG) && (ByteOne < MOV_MEM_TO_ACCUM) )
    {
        Debug_OutputErrorMessage("MOV_REGorMEM_TO_SEGREG");
        // Not yet
    }

    else if( (ByteOne >= MOV_MEM_TO_ACCUM) && (ByteOne < MOV_ACCUM_TO_MEM) )
    {
        DecodedInst->Size = 3;

        if(ParsedInst.IsWord)
        {
            SteenCopy(&DecodedReg, "ax", 2);
        }
        else
        {
            SteenCopy(&DecodedReg, "al", 2);
        }

        SteenCopy(&DecodedRorM, "[", 1);
        struct int_to_print DirectAddress = {0};
        InitString(&DirectAddress.String);
        DirectAddress.Type = UNSIGNED_16;
        DirectAddress.Data.U16 = *(u16 *)(ParsedInst.Binary + 1); 
        GetIntAsChars(&DirectAddress);

        SteenCopy(&DecodedRorM, (char *)DirectAddress.String.String, DirectAddress.String.Len);
        SteenCopy(&DecodedRorM, "]", 1);
        SteenCopy(&DecodedInst->OperandOne, DecodedReg.String, DecodedReg.Len);
        SteenCopy(&DecodedInst->OperandTwo, DecodedRorM.String, DecodedRorM.Len);
    }

    else if( (ByteOne >= MOV_ACCUM_TO_MEM) && (ByteOne < MOV_IMM_TO_REG) )
    {
        DecodedInst->Size = 3;

        if(ParsedInst.IsWord)
        {
            SteenCopy(&DecodedReg, "ax", 2);
        }
        else
        {
            SteenCopy(&DecodedReg, "al", 2);
        }

        SteenCopy(&DecodedRorM, "[", 1);

        struct int_to_print DirectAddress = {0};
        InitString(&DirectAddress.String);
        DirectAddress.Type = UNSIGNED_16;
        DirectAddress.Data.U16 = *(u16 *)(ParsedInst.Binary + 1); 
        GetIntAsChars(&DirectAddress);
        SteenCopy(&DecodedRorM, (char *)DirectAddress.String.String, DirectAddress.String.Len);
        SteenCopy(&DecodedRorM, "]", 1);

        SteenCopy(&DecodedInst->OperandOne, DecodedRorM.String, DecodedRorM.Len);	
        SteenCopy(&DecodedInst->OperandTwo, DecodedReg.String, DecodedReg.Len);
    }

    else if( (ByteOne >= MOV_IMM_TO_REG) && (ByteOne < MOV_IMM_TO_REGorMEM) )
    {
        ParsedInst.IsWord = (bool)((ByteOne & 0x08) >> 3);
        ParsedInst.Reg = (ByteOne & 0x07);

        char *WhichRegIsIt = DecodeReg(ParsedInst.IsWord, ParsedInst.Reg);
        SteenCopy(&DecodedReg, WhichRegIsIt, REGISTER_NAME_LEN);

        struct int_to_print ImmValue = {0};
        InitString(&ImmValue.String);

        if(ParsedInst.IsWord)
        {
            ImmValue.Type = SIGNED_16;
            ImmValue.Data.S16 = *(s16 *)(ParsedInst.Binary + 1);	    
            DecodedInst->Size = 3;
        }
        else
        {
            ImmValue.Type = SIGNED_8;
            ImmValue.Data.S8 = *(s8 *)(ParsedInst.Binary + 1);	    
            DecodedInst->Size = 2;
        }
        GetIntAsChars(&ImmValue);
        SteenCopy(&DecodedInst->OperandOne, DecodedReg.String, DecodedReg.Len);
        SteenCopy(&DecodedInst->OperandTwo, ImmValue.String.String, ImmValue.String.Len);
    }
    else if( (ByteOne >= MOV_IMM_TO_REGorMEM) && (ByteOne <= (MOV_IMM_TO_REGorMEM + 1)) )
    {
        DecodeRorM(&ParsedInst, &DecodedRorM, DecodedInst);

        struct int_to_print ImmValue = {0};
        InitString(&ImmValue.String);
        struct string ExplicitSize = {0};
        InitString(&ExplicitSize);
        int NumDispBytes = 0;


        DecodedInst->Size = 3;
        if(ParsedInst.Mod == MEM_MODE_DISP8)
        {
            DecodedInst->Size += 1;
            NumDispBytes += 1;

        }
        else if(ParsedInst.Mod == MEM_MODE_DISP16)
        {
            DecodedInst->Size += 2;
            NumDispBytes += 2;
        }
        else
        {
            Debug_OutputErrorMessage("Couldn't decode Mod field for MOV_IMM_TO_REGorMEM case");
        }

        if(ParsedInst.IsWord)
        {
            DecodedInst->Size += 1;
            SteenCopy(&ExplicitSize, "word ", 5);
            ImmValue.Type = SIGNED_16;
            ImmValue.Data.S16 = *(s16 *)(ParsedInst.Binary + NumDispBytes + 2);
        }
        else
        {	    
            SteenCopy(&ExplicitSize, "byte ", 5);
            ImmValue.Type = SIGNED_8;
            ImmValue.Data.S8 = *(s8 *)(ParsedInst.Binary + NumDispBytes + 2);
        }

        GetIntAsChars(&ImmValue);
        SteenCopy(&ExplicitSize, ImmValue.String.String, ImmValue.String.Len);

        SteenCopy(&DecodedInst->OperandOne, DecodedRorM.String, DecodedRorM.Len);
        SteenCopy(&DecodedInst->OperandTwo, ExplicitSize.String, ExplicitSize.Len);
    }
    else
    {
        // Error
        exit(1);
    }

}

internal void 
DecodePush(struct decoded_inst *DecodedInst)
{
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    struct parsed_inst ParsedInst = {0};
    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.Disp8 = *(s8 *)DecodedInst->Binary + 2;
    ParsedInst.Disp16 = *(s16 *)DecodedInst->Binary + 2;
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    struct string DecodedReg = {0};
    InitString(&DecodedReg);
    struct string DecodedRorM = {0};
    InitString(&DecodedRorM);
    struct string Operand = {0};
    InitString(&Operand);

    if( (ByteOne >= PUSH_SEG_REG) && (ByteOne < PUSH_REG) )
    {
        DecodedInst->Size = 1;
        ParsedInst.Reg = ((ByteOne & CHECK_SEG_REG) >> 3);
        SteenCopy(&Operand, SegRegLUT[ParsedInst.Reg], REGISTER_NAME_LEN);
    }

    else if( (ByteOne >= PUSH_REG) && (ByteOne < PUSH_REG_OR_MEM) )
    {
        DecodedInst->Size = 1;
        ParsedInst.Reg = (ByteOne & 0x03);
        char *WhichRegIsIt = DecodeReg(1, ParsedInst.Reg);
        SteenCopy(&Operand, WhichRegIsIt, REGISTER_NAME_LEN);
    }

    else if(ByteOne >= PUSH_REG_OR_MEM)
    {
        DecodedInst->Size = 2;

        SteenCopy(&Operand, "word ", 5); 
        if(ParsedInst.Mod == MEM_MODE_DISP8)
        {
            DecodedInst->Size += 1;
        }
        else if(ParsedInst.Mod == MEM_MODE_DISP16)
        {
            DecodedInst->Size += 2;
        }
        DecodeRorM(&ParsedInst, &DecodedRorM, DecodedInst);
        SteenCopy(&Operand, DecodedRorM.String, DecodedRorM.Len);
    }
    else
    {
        // Error
        exit(1);
    }
    SteenCopy(&DecodedInst->OperandOne, Operand.String, Operand.Len);
}

internal void 
DecodePop(struct decoded_inst *DecodedInst)
{
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    struct parsed_inst ParsedInst = {0};
    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.Disp8 = *(s8 *)DecodedInst->Binary + 2;
    ParsedInst.Disp16 = *(s16 *)DecodedInst->Binary + 2;
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    struct string DecodedReg = {0};
    InitString(&DecodedReg);
    struct string DecodedRorM = {0};
    InitString(&DecodedRorM);
    struct string Operand = {0};
    InitString(&Operand);

    if( (ByteOne >= POP_SEG_REG) && (ByteOne < POP_REG) )
    {
        DecodedInst->Size = 1;
        ParsedInst.Reg = ((ByteOne & CHECK_SEG_REG) >> 3);
        SteenCopy(&Operand, SegRegLUT[ParsedInst.Reg], REGISTER_NAME_LEN);
    }

    else if( (ByteOne >= POP_REG) && (ByteOne < POP_REG_OR_MEM) )
    {
        DecodedInst->Size = 1;
        ParsedInst.Reg = (ByteOne & 0x07);
        char *WhichRegIsIt = DecodeReg(1, ParsedInst.Reg);
        SteenCopy(&Operand, WhichRegIsIt, REGISTER_NAME_LEN);
    }

    else if(ByteOne >= POP_REG_OR_MEM)
    {
        DecodedInst->Size = 2;

        SteenCopy(&Operand, "word ", 5); 
        if(ParsedInst.Mod == MEM_MODE_DISP8)
        {
            DecodedInst->Size += 1;
        }
        else if(ParsedInst.Mod == MEM_MODE_DISP16)
        {
            DecodedInst->Size += 2;
        }
        DecodeRorM(&ParsedInst, &DecodedRorM, DecodedInst);
        SteenCopy(&Operand, DecodedRorM.String, DecodedRorM.Len);
    }
    else
    {
        // Error
        exit(1);
    }
    SteenCopy(&DecodedInst->OperandOne, Operand.String, Operand.Len);
}

internal void
DecodeXchg(struct decoded_inst *DecodedInst)
{
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    struct parsed_inst ParsedInst = {0};
    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.Disp8 = *(s8 *)DecodedInst->Binary + 2;
    ParsedInst.Disp16 = *(s16 *)DecodedInst->Binary + 2;
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);
    ParsedInst.IsWord = (bool)(ByteOne & 0x01);

    struct string DecodedReg = {0};
    InitString(&DecodedReg);
    struct string DecodedRorM = {0};
    InitString(&DecodedRorM);

    if( (ByteOne >= XCHG_REG_or_MEM_WITH_REG) && (ByteOne < XCHG_REG_WITH_ACCUM) )
    {
        DecodedInst->Size = 2;

        char *WhichRegIsIt = DecodeReg(ParsedInst.IsWord, ParsedInst.Reg);
        SteenCopy(&DecodedReg, WhichRegIsIt, REGISTER_NAME_LEN);
        DecodeRorM(&ParsedInst, &DecodedRorM, DecodedInst);

        if(ParsedInst.Mod == MEM_MODE_DISP8)
        {
            DecodedInst->Size += 1;
        }
        else if(ParsedInst.Mod == MEM_MODE_DISP16)
        {
            DecodedInst->Size += 2;
        }


        // Assume reg is dest
        SteenCopy(&DecodedInst->OperandOne, DecodedReg.String, DecodedReg.Len);	
        SteenCopy(&DecodedInst->OperandTwo, DecodedRorM.String, DecodedRorM.Len); 
        1;
    }

    else if( (ByteOne >= XCHG_REG_WITH_ACCUM) && (ByteOne <= XCHG_UPPER_LIMIT) ) 
    {
        ParsedInst.Reg = (ByteOne & 0x07);
        ParsedInst.IsWord = true;

        if(DecodedInst->OpcodeEnum == NOP)
        {
            DecodedInst->Mnemonic = "xchg";
        }
        DecodedInst-> Size = 1;

        SteenCopy(&DecodedInst->OperandOne, "ax", REGISTER_NAME_LEN);

        // we have to override the word flag this instruction doesn't use it
        //    and it can be set erroneously
        ParsedInst.IsWord = true;
        char *WhichRegIsIt = DecodeReg(ParsedInst.IsWord, ParsedInst.Reg);

        SteenCopy(&DecodedInst->OperandTwo, WhichRegIsIt, REGISTER_NAME_LEN);

        1;
    }

    else
    {

        Debug_PrintCurrentStatus(DecodedInst);
        Debug_OutputErrorMessage("Couldn't decode ByteOne of Xchg instruction");
        exit(1);
    }
}

internal void
DecodeInOrOut(struct decoded_inst *DecodedInst)
{
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    struct parsed_inst ParsedInst = {0};
    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.IsWord = (ByteOne & 0x01);
    ParsedInst.DestFlag = (bool)((ByteOne & 0x02) >> 1); 
    struct string Op1 = {0};
    struct string Op2 = {0};

    if( (ByteOne >= IN_FIXED_PORT) && (ByteOne < IN_VARIABLE_PORT_BYTE) ) 
    {
        DecodedInst->Size = 2;
        struct string *ImmOp;
        struct string *Reg;

        // if dest flag it's an out instruction
        if(ParsedInst.DestFlag)
        {
            ImmOp = &Op1;
            Reg = &Op2;
        }
        else
        {
            Reg = &Op1;
            ImmOp = &Op2;
        }
        InitString(Reg);
        InitString(ImmOp);

        if(ParsedInst.IsWord)
        {
            SteenCopy(Reg, "ax", 2);
        }
        else
        {
            SteenCopy(Reg, "al", 2);
        }

        // immediate value
        struct int_to_print ImmVal = {0};
        InitString(&ImmVal.String);
        ImmVal.Type = UNSIGNED_8;
        ImmVal.Data.U8 = (u8)ByteTwo;
        GetIntAsChars(&ImmVal);

        SteenCopy(ImmOp, ImmVal.String.String, ImmVal.String.Len);
    }

    else if( (ByteOne >= IN_VARIABLE_PORT_BYTE) && (ByteOne <= OUT_VARIABLE_PORT_WORD) )
    {
        DecodedInst->Size = 1;
        struct string *Reg;
        struct string *Accum;

        // if dest flag it's an out instruction
        if(ParsedInst.DestFlag)
        {
            Reg = &Op1;
            Accum = &Op2;
        }
        else
        {
            Accum = &Op1;
            Reg = &Op2;
        }
        InitString(Accum);
        InitString(Reg);

        if(ParsedInst.IsWord)
        {
            SteenCopy(Accum, "ax", 2);
        }
        else
        {
            SteenCopy(Accum, "al", 2);
        }

        // Reg is always dx
        SteenCopy(Reg, "dx", 2);
    }

    else
    {
        Debug_OutputErrorMessage("Couldn't decode In instruction");
        exit(1);
    }

    WriteOperands(DecodedInst, &Op1, &Op2);
}

internal void
DecodeLoad(struct decoded_inst *DecodedInst)
{
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    struct parsed_inst ParsedInst = {0};
    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    struct string Op1 = {0};
    InitString(&Op1);
    struct string Op2 = {0};
    InitString(&Op2);

    DecodedInst->Size = 2;

    if(ParsedInst.Mod == MEM_MODE_DISP8)
    {
        DecodedInst->Size = 3;
    }
    else if(ParsedInst.Mod == MEM_MODE_DISP16)
    {
        DecodedInst->Size = 4;
    }
    char *Reg = WordRegLUT[ParsedInst.Reg]; 
    SteenCopy(&Op1, Reg, GetZTermStringLen(Reg));

    struct string DecodedRorM = {0};
    InitString(&DecodedRorM);
    DecodeRorM(&ParsedInst, &DecodedRorM, DecodedInst);
    SteenCopy(&Op2, DecodedRorM.String, DecodedRorM.Len);

    WriteOperands(DecodedInst, &Op1, &Op2);
}

internal void
DecodeArithmetic(struct decoded_inst *DecodedInst)
{
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    struct parsed_inst ParsedInst = {0};
    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.DestFlag = ((ByteOne & 0x02) >> 1);
    ParsedInst.IsWord = (ByteOne & 0x01);
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    struct string Op1 = {0};
    struct string Op2 = {0};

    int BitSeven = ((ByteOne & 0x80) >> 7);
    int BitThree = ((ByteOne & 0x04) >> 2);

    // imm to reg or mem
    if(BitSeven)
    {
        DecodedInst->Size = 3;
        bool SignExtend = ((ByteOne & 0x02) >> 1);

        struct string *DecodedRorM = &Op1;
        InitString(DecodedRorM);
        struct int_to_print ImmValue = {0};
        InitString(&ImmValue.String);
        int NumDispBytes = 0;

        DecodeRorM(&ParsedInst, DecodedRorM, DecodedInst);

        if(ParsedInst.Mod == MEM_MODE_DISP8)
        {
            DecodedInst->Size += 1;
            NumDispBytes += 1;

        }
        else if(ParsedInst.Mod == MEM_MODE_DISP16)
        {
            DecodedInst->Size += 2;
            NumDispBytes += 2;
        }

        if(ParsedInst.IsWord)
        {
            if(SignExtend)
            {
                ImmValue.Type = SIGNED_16;
                s8 Temp = *(s8 *)(ParsedInst.Binary + NumDispBytes + 2);
                ImmValue.Data.S16 = (s16)Temp;
            }
            else
            {
                DecodedInst->Size += 1;
                ImmValue.Type = SIGNED_16;
                ImmValue.Data.S16 = *(s16 *)(ParsedInst.Binary + NumDispBytes + 2);
            }
        }
        else
        {
            ImmValue.Type = SIGNED_8;
            ImmValue.Data.S8 = *(s8 *)(ParsedInst.Binary + NumDispBytes + 2);
        }

        GetIntAsChars(&ImmValue);
        Op2 = ImmValue.String;

    }

    // imm to accum
    else if(BitThree)
    {
        struct string *DecodedReg = &Op1;
        InitString(DecodedReg);
        struct int_to_print ImmValue = {0};
        InitString(&ImmValue.String);

        if(ParsedInst.IsWord)
        {
            SteenCopy(DecodedReg, "ax", 2);

            ImmValue.Type = SIGNED_16;
            ImmValue.Data.S16 = *(s16 *)(ParsedInst.Binary + 1);	    
            DecodedInst->Size = 3;
        }
        else
        {
            SteenCopy(DecodedReg, "al", 2);

            ImmValue.Type = SIGNED_8;
            ImmValue.Data.S8 = *(s8 *)(ParsedInst.Binary + 1);	    
            DecodedInst->Size = 2;
        }
        GetIntAsChars(&ImmValue);
        Op2 = ImmValue.String;
    }

    // reg/mem with reg to either
    else if (BitThree == 0)
    {
        DecodedInst->Size = 2;
        struct string *DecodedReg;
        struct string *DecodedRorM;
        if(ParsedInst.DestFlag)
        {
            DecodedReg = &Op1;
            DecodedRorM = &Op2;
        }
        else
        {
            DecodedRorM = &Op1;
            DecodedReg = &Op2;
        }
        InitString(DecodedReg);
        InitString(DecodedRorM);

        OmniDecode(DecodedInst, &ParsedInst, DecodedReg, DecodedRorM);
    }
    else
    {
        Debug_OutputErrorMessage("Couldn't decode arithmetic instruction");
        exit(1);
    }

    WriteOperands(DecodedInst, &Op1, &Op2);
}

internal void
DecodeIncOrDec(struct decoded_inst *DecodedInst)
{
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    struct parsed_inst ParsedInst = {0};
    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.DestFlag = ((ByteOne & 0x02) >> 1);
    ParsedInst.IsWord = (ByteOne & 0x01);
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    struct string Operand;
    InitString(&Operand);


    // reg or mem
    if( (ByteOne == 0xFE) || (ByteOne == 0xFF) )
    {
        struct string *DecodedRorM = &Operand;
        DecodedInst->Size = 2;

        if(ParsedInst.Mod != 3)
        {
            if(ParsedInst.IsWord)
            {
                SteenCopy(DecodedRorM, "word ", 5);
            }
            else
            {
                SteenCopy(DecodedRorM, "byte ", 5);
            }
        }
        DecodeRorM(&ParsedInst, DecodedRorM, DecodedInst);
        if(ParsedInst.Mod == MEM_MODE_DISP8)
        {
            DecodedInst->Size += 1;
        }
        else if(ParsedInst.Mod == MEM_MODE_DISP16)
        {
            DecodedInst->Size += 2;
        }
    }

    // reg
    else if( (ByteOne >= 0x40) && (ByteOne <= 0x4F) )
    {
        DecodedInst->Size = 1;
        int RegCode = (ByteOne & 0x07);
        char *WhichRegIsIt = WordRegLUT[RegCode];
        SteenCopy(&Operand, WhichRegIsIt, GetZTermStringLen(WhichRegIsIt));
    }

    else
    {
        Debug_OutputErrorMessage("Couldn't decode inc/dec instruction");
        exit(1);
    }

    WriteOperands(DecodedInst, &Operand, NULL);

}

internal void
DecodeGroup3(struct decoded_inst *DecodedInst)
{

    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    struct parsed_inst ParsedInst = {0};
    ParsedInst.Binary = DecodedInst->Binary;
    ParsedInst.IsWord = (bool)(ByteOne & 0x01);
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    struct string DecodedRorM = {0};
    InitString(&DecodedRorM);

    ParsedInst.Disp8 = *(s8 *)DecodedInst->Binary + 2;
    ParsedInst.Disp16 = *(s16 *)DecodedInst->Binary + 2;
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    DecodedInst->Size = 2;

    if(ParsedInst.Mod != REG_MODE)
    {
        if(ParsedInst.IsWord)
        {
            SteenCopy(&DecodedInst->OperandOne, "word ", 5); 
        }
        else
        {
            SteenCopy(&DecodedInst->OperandOne, "byte ", 5); 
        }
        
        if(ParsedInst.Mod == MEM_MODE_DISP8)
        {
            DecodedInst->Size += 1;
        }
        if(ParsedInst.Mod == MEM_MODE_DISP16)
        {
            DecodedInst->Size += 2;
        }
    }
    
    DecodeRorM(&ParsedInst, &DecodedRorM, DecodedInst);
    SteenCopy(&DecodedInst->OperandOne, DecodedRorM.String, DecodedRorM.Len);

}



internal u8
ReadExtendedOpcode(struct decoded_inst *DecodedInst)
{
    u8 FirstByte = DecodedInst->Binary[0];
    u8 MeetTheDistinguisher = 0x38; //0011 1000
    u8 IHaveMetTheDistinguisher = DecodedInst->Binary[1] & MeetTheDistinguisher;

    switch(FirstByte)
    {
        case 0x80:
        case 0x81:
            {
                switch(IHaveMetTheDistinguisher)
                {
                    case 0x00: // 000
                        {
                            return(ADD);
                        } break;

                    case 0x08: // 001
                        {
                            return(OR);
                        } break;

                    case 0x10: // 010
                        {
                            return(ADC);
                        } break;

                    case 0x18: // 011
                        {
                            return(SBB);
                        } break;

                    case 0x20: // 100
                        {
                            return(AND);
                        } break;

                    case 0x28: // 101
                        {
                            return(SUB);
                        } break;

                    case 0x30: // 110
                        {
                            return(XOR);
                        } break;

                    case 0x38: // 111
                        {
                            return(CMP);
                        } break;

                    default:
                        {
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0x80 or 0x81:\n    ADD, OR, ADC, SBB, AND, SUB, XOR, or CMP");
                            exit(1);
                        } break;
                }
            } break;

        case 0x82:
            {
                switch(IHaveMetTheDistinguisher)
                {
                    case 0x00: // 000
                        {
                            return(ADD);
                        } break;

                    case 0x10: // 010
                        {
                            return(ADC);
                        } break;

                    case 0x18: // 011
                        {
                            return(SBB);
                        } break;

                    case 0x28: // 101
                        {
                            return(SUB);
                        } break;

                    case 0x38: // 111
                        {
                            return(CMP);
                        } break;

                    default:
                        {
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0x82:\n    ADD, ADC, SBB, SUB or CMP");
                            exit(1);
                        } break;
                }
            } break;

        case 0x83:
            {
                switch(IHaveMetTheDistinguisher)
                {
                    case 0x00: // 000
                        {
                            return(ADD);
                        } break;

                    case 0x10: // 010
                        {
                            return(ADC);
                        } break;

                    case 0x18: // 011
                        {
                            return(SBB);
                        } break;

                    case 0x28: // 101
                        {
                            return(SUB);
                        } break;

                    case 0x38: // 111
                        {
                            return(CMP);
                        } break;

                    default:
                        {
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0x83:\n    ADD, ADC, SBB, SUB or CMP");
                            exit(1);
                        } break;
                }
            } break;

        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
            {
                switch(IHaveMetTheDistinguisher)
                {
                    case 0x00: // 000
                        {
                            return(ROL);
                        } break;

                    case 0x08: // 001
                        {
                            return(ROR);
                        } break;

                    case 0x10: // 010
                        {
                            return(RCL);
                        } break;

                    case 0x18: // 011
                        {
                            return(RCR);
                        } break;

                    case 0x20: // 100
                        {
                            return(SHL);
                        } break;

                    case 0x28: // 101
                        {
                            return(SHR);
                        } break;

                    case 0x38: // 111
                        {
                            return(SAR);
                        } break;

                    default:
                        {
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0xD0, 0xD1, 0xD2, 0xD3:\n    ROL, ROR, RCL, RCR, TBD_SHL, SHR, or SAR");
                            exit(1);
                        } break;
                }
            } break;

        case 0xF6:
        case 0xF7:
            {
                switch(IHaveMetTheDistinguisher)
                {
                    case 0x00: // 000
                        {
                            return(TEST);
                        } break;

                    case 0x10: // 010
                        {
                            return(NOT);
                        } break;

                    case 0x18: // 011
                        {
                            return(NEG);
                        } break;

                    case 0x20: // 100
                        {
                            return(MUL);
                        } break;

                    case 0x28: // 101
                        {
                            return(IMUL);
                        } break;

                    case 0x30: // 110
                        {
                            return(DIV);
                        } break;

                    case 0x38: // 111
                        {
                            return(IDIV);
                        } break;

                    default:
                        { 
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0xF6 or 0xF7:\n   TEST, NOT, NEG, MUL, IMUL, DIV or IDIV"); 
                            exit(1);
                        }
                }
            }

        case 0xFE:
            {
                switch(IHaveMetTheDistinguisher)
                {
                    case 0x00: // 000
                        {
                            return(INC);
                        } break;

                    case 0x08: // 010
                        {
                            return(DEC);
                        } break;

                    default:
                        {
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0xFE:\n   INC or DEC"); 
                            exit(1);
                        } break;
                }
            } break;

        case 0xFF:
            {
                switch(IHaveMetTheDistinguisher)
                {
                    case 0x00: // 000
                        {
                            return(INC);
                        } break;

                    case 0x08: // 001
                        {
                            return(DEC);
                        } break;

                    case 0x10: // 010
                    case 0x18: // 011
                        {
                            return(CALL);
                        } break;

                    case 0x20: // 100
                    case 0x28: // 101
                        {
                            return(JMP);
                        } break;

                    case 0x30: // 110
                        {
                            return(PUSH);
                        } break;

                    default:
                        {
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0xFF:\n   INC, DEC, CALL, JMP or PUSH"); 
                            exit(1);
                        } break;
                }
            } break;

        default:
            {
                Debug_OutputErrorMessage("Failed to decode extended opcode");
                exit(1);
            } break;
    }
}

internal void
Dispatch(struct decoded_inst *DecodedInst)
{
    switch(DecodedInst->OpcodeEnum)
    {
        case(MOV):
            {
                DecodeMov(DecodedInst);
            } break;

        case(PUSH):
            {
                DecodePush(DecodedInst);
            } break;

        case(POP):
            {
                DecodePop(DecodedInst);
            } break;

        case(XCHG):
        case(NOP):
            {
                DecodeXchg(DecodedInst);
            } break;

        case(IN_):
        case(OUT_):
            {
                DecodeInOrOut(DecodedInst);
            } break;

        case(LEA):
        case(LDS):
        case(LES):
            {
                DecodeLoad(DecodedInst);
            } break;

        case(ADD):
        case(ADC):
        case(SUB):
        case(SBB):
        case(CMP):
            {
                DecodeArithmetic(DecodedInst);
            } break;

        case(INC):
        case(DEC):
            {
                DecodeIncOrDec(DecodedInst);
            } break;

        case(XLAT): 
        case(LAHF):
        case(SAHF):
        case(PUSHF):
        case(POPF):
        case(AAA):
        case(DAA):
        case(AAS):
        case(DAS):
        case(CBW):
        case(CWD):
        case(INTO):
        case(IRET):
        case(CLC):
        case(CMC):
        case(STC):
        case(CLD):
        case(STD):
        case(CLI):
        case(STI):
        case(HLT):
        case(WAIT):
        case(RET):
            {
                DecodedInst->Size = 1;
                return;
            } break;

        case(AAM):
        case(AAD):
            {
                DecodedInst->Size = 2;
                return;
            } break;

        case(NEG):
        case(NOT):
            
            {
                DecodeGroup3(DecodedInst);
                return;
            } break;

        case(MUL):
        case(IMUL):
        case(DIV):
        case(IDIV):
            {
                DecodeGroup3(DecodedInst);
                return;
            } break;

        case(SHL):
        case(SHR):
        case(SAR):
        case(ROL):
        case(ROR):
        case(RCL):
        case(RCR):
        case(RCR):
            {
                DecodeGroup3(DecodedInst);

                bool VFlag = ((DecodedInst->Binary[0] & 0x02) >> 1);
                if(VFlag)
                {
                    SteenCopy(DecodedInst->OperandTwo, "cl", 2);
                }
                else
                {
                    SteenCopy(DecodedInst->OperandTwo, "1", 1);
                }
                return;
            } break;



        default:
            {
                // Error
                Debug_PrintCurrentStatus(DecodedInst);
                Debug_OutputErrorMessage("No matching dispatch function");
                exit(1);
            } break;
    }
}



    OutputDebugStringA(".\n\n");
}

inter
