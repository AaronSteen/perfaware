#include "sim.h"


internal void
SteenCopy(struct string *Dest, char *Source, SIZE_T NumBytes)
{
    for(int i = 0; i < NumBytes; i++)
    {
	*Dest->Ptr++ = *Source++;
	Dest->Len++;
    }
}

internal char * 
GetEffectiveAddress(u8 RorMField)
{
    return(EffectiveAddressLUT[RorMField]);
}

internal int
Get8BitsAsChars(char *PrintBuffer, s8 Bits)
{
    int BytesWritten = sprintf_s(PrintBuffer, sizeof(PrintBuffer), "%d", Bits);
    return(BytesWritten);
}

internal int
Get16BitsAsChars(char *PrintBuffer, s16 Bits)
{
    int BytesWritten = sprintf_s(PrintBuffer, sizeof(PrintBuffer), "%d", Bits);
    return(BytesWritten);
}


internal void
WriteToOutBuf(char *Source, SIZE_T NumBytes)
{
    if((u8 *)(OutBuf.Ptr + NumBytes) >= OutBuf.DoNotCrossThisLine)
    {
	Debug_OutputErrorMessage("OutBuf overflow", __func__, __LINE__);
	exit(1);
    }

    for(int i = 0; i < NumBytes; i++)
    {
	*OutBuf.Ptr++ = *Source++;
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
DecodeMov(struct decoded_inst *DecodedInst)
{
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];
    struct parsed_inst ParsedMov = {0};
    ParsedMov.Binary = DecodedInst->Binary;
    ParsedMov.DestFlag = (bool)((ByteOne & 0x02) >> 1);
    ParsedMov.IsWord = (bool)(ByteOne & 0x01);
    ParsedMov.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedMov.Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedMov.RorM = (ByteTwo & R_OR_M_FIELD);

    struct string DecodedReg = {0};
    DecodedReg.Ptr = DecodedReg.String;
    struct string DecodedRorM = {0};
    DecodedRorM.Ptr = DecodedRorM.String;

    if( (ByteOne >= MOV_MEMorREG_TOorFROM_REG) && (ByteOne < MOV_SEGREG_TO_REGorMEM) )
    {
	DecodedInst->Size = 2;

	char *WhichRegIsIt = DecodeReg(ParsedMov.IsWord, ParsedMov.Reg);
	SteenCopy(&DecodedReg, WhichRegIsIt, REGISTER_NAME_LEN);

	DecodeRorM(&ParsedMov, &DecodedRorM, DecodedInst);
	if(ParsedMov.Mod == MEM_MODE_DISP8)
	{
	    DecodedInst->Size = 3;
	}
	else if( (ParsedMov.Mod == MEM_MODE_DISP16) ||
		    (ParsedMov.Mod == MEM_MODE_NO_DISP && ParsedMov.RorM == DIRECT_ADDRESS) )
	{
	    DecodedInst->Size = 4;
	}
	if(ParsedMov.DestFlag)
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
	Debug_OutputErrorMessage("MOV_SEGREG_TO_REGorMEM", __func__, __LINE__);
	// Not yet
    }
    
    else if( (ByteOne >= MOV_REGorMEM_TO_SEGREG) && (ByteOne < MOV_MEM_TO_ACCUM) )
    {
	Debug_OutputErrorMessage("MOV_REGorMEM_TO_SEGREG", __func__, __LINE__);
	// Not yet
    }

    else if( (ByteOne >= MOV_MEM_TO_ACCUM) && (ByteOne < MOV_ACCUM_TO_MEM) )
    {
	DecodedInst->Size = 3;

	if(ParsedMov.IsWord)
	{
	    SteenCopy(&DecodedReg, "ax", 2);
	}
	else
	{
	    SteenCopy(&DecodedReg, "al", 2);
	}

	SteenCopy(&DecodedRorM, "[", 1);
	char DirectAddressBuffer[MAX_STRING_LEN];
	s16 DirectAddress = *(s16 *)(ParsedMov.Binary + 1);
	int NumDirectAddrChars = Get16BitsAsChars(DirectAddressBuffer, DirectAddress);
	SteenCopy(&DecodedRorM, DirectAddressBuffer, NumDirectAddrChars);
	SteenCopy(&DecodedRorM, "]", 1);

	SteenCopy(&DecodedInst->OperandOne, DecodedReg.String, DecodedReg.Len);
	SteenCopy(&DecodedInst->OperandTwo, DecodedRorM.String, DecodedRorM.Len);
    }

    else if( (ByteOne >= MOV_ACCUM_TO_MEM) && (ByteOne < MOV_IMM_TO_REG) )
    {
	DecodedInst->Size = 3;

	if(ParsedMov.IsWord)
	{
	    SteenCopy(&DecodedReg, "ax", 2);
	}
	else
	{
	    SteenCopy(&DecodedReg, "al", 2);
	}

	SteenCopy(&DecodedRorM, "[", 1);
	char DirectAddressBuffer[MAX_STRING_LEN];
	s16 DirectAddress = *(s16 *)(ParsedMov.Binary + 1);
	int NumDirectAddrChars = Get16BitsAsChars(DirectAddressBuffer, DirectAddress);
	SteenCopy(&DecodedRorM, DirectAddressBuffer, NumDirectAddrChars);
	SteenCopy(&DecodedRorM, "]", 1);

	SteenCopy(&DecodedInst->OperandOne, DecodedRorM.String, DecodedRorM.Len);	
	SteenCopy(&DecodedInst->OperandTwo, DecodedReg.String, DecodedReg.Len);
    }

    else if( (ByteOne >= MOV_IMM_TO_REG) && (ByteOne < MOV_IMM_TO_REGorMEM) )
    {
	ParsedMov.IsWord = (bool)((ByteOne & 0x08) >> 3);
	ParsedMov.Reg = (ByteOne & 0x07);

	char *WhichRegIsIt = DecodeReg(ParsedMov.IsWord, ParsedMov.Reg);
	SteenCopy(&DecodedReg, WhichRegIsIt, REGISTER_NAME_LEN);

	char ImmediateValueBuffer[MAX_STRING_LEN];
	int NumValueChars = 0;

	if(ParsedMov.IsWord)
	{
	    s16 ImmediateValue = *(s16 *)(ParsedMov.Binary + 1);
	    NumValueChars = Get16BitsAsChars(ImmediateValueBuffer, ImmediateValue);
	    DecodedInst->Size = 3;
	}
	else
	{
	    s8 ImmediateValue = *(s8 *)(ParsedMov.Binary + 1);
	    NumValueChars = Get8BitsAsChars(ImmediateValueBuffer, ImmediateValue);
	    DecodedInst->Size = 2;
	}

	SteenCopy(&DecodedInst->OperandOne, DecodedReg.String, DecodedReg.Len);
	SteenCopy(&DecodedInst->OperandTwo, ImmediateValueBuffer, NumValueChars);
    }
    else if( (ByteOne >= MOV_IMM_TO_REGorMEM) && (ByteOne <= (MOV_IMM_TO_REGorMEM + 1)) )
    {
	DecodeRorM(&ParsedMov, &DecodedRorM, DecodedInst);
	int NumDispBytes = 0;
	DecodedInst->Size = 3;

	if(ParsedMov.Mod == MEM_MODE_DISP8)
	{
	    DecodedInst->Size += 1;
	    NumDispBytes = 1;
	}
	else if(ParsedMov.Mod == MEM_MODE_DISP16)
	{
	    DecodedInst->Size += 2;
	    NumDispBytes = 2;
	}

	struct string ExplicitSize = {0};
	ExplicitSize.Ptr = ExplicitSize.String;
	char ImmediateValueBuffer[MAX_STRING_LEN];
	int NumImmediateChars = 0;

	if(ParsedMov.IsWord)
	{
	    DecodedInst->Size += 1;
	    SteenCopy(&ExplicitSize, "word ", 5);
	    s16 ImmediateValue = *(s16 *)(ParsedMov.Binary + NumDispBytes + 2);
	    NumImmediateChars = Get16BitsAsChars(ImmediateValueBuffer, ImmediateValue);
	    SteenCopy(&ExplicitSize, ImmediateValueBuffer, NumImmediateChars);
	}
	else
	{	    
	    SteenCopy(&ExplicitSize, "byte ", 5);
	    s8 ImmediateValue = *(s8 *)(ParsedMov.Binary + NumDispBytes + 2);
	    NumImmediateChars = Get8BitsAsChars(ImmediateValueBuffer, ImmediateValue);
	    SteenCopy(&ExplicitSize, ImmediateValueBuffer, NumImmediateChars);
	}

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

    struct parsed_inst ParsedPush = {0};
    ParsedPush.Binary = DecodedInst->Binary;
    ParsedPush.Disp8 = *(s8 *)DecodedInst->Binary + 2;
    ParsedPush.Disp16 = *(s16 *)DecodedInst->Binary + 2;
    ParsedPush.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedPush.RorM = (ByteTwo & R_OR_M_FIELD);

    struct string DecodedReg = {0};
    DecodedReg.Ptr = DecodedReg.String;
    struct string DecodedRorM = {0};
    DecodedRorM.Ptr = DecodedRorM.String;
    struct string Operand = {0};
    Operand.Ptr = Operand.String;

    if( (ByteOne >= PUSH_SEG_REG) && (ByteOne < PUSH_REG) )
    {
	DecodedInst->Size = 1;
	ParsedPush.Reg = ((ByteOne & CHECK_SEG_REG) >> 3);
	SteenCopy(&Operand, SegRegLUT[ParsedPush.Reg], REGISTER_NAME_LEN);
    }

    else if( (ByteOne >= PUSH_REG) && (ByteOne < PUSH_REG_OR_MEM) )
    {
	DecodedInst->Size = 1;
	ParsedPush.Reg = (ByteOne & 0x03);
	char *WhichRegIsIt = DecodeReg(1, ParsedPush.Reg);
	SteenCopy(&Operand, WhichRegIsIt, REGISTER_NAME_LEN);
    }

    else if(ByteOne >= PUSH_REG_OR_MEM)
    {
	DecodedInst->Size = 2;

	SteenCopy(&Operand, "word ", 5); 
	if(ParsedPush.Mod == MEM_MODE_DISP8)
	{
	    DecodedInst->Size += 1;
	}
	else if(ParsedPush.Mod == MEM_MODE_DISP16)
	{
	    DecodedInst->Size += 2;
	}
	DecodeRorM(&ParsedPush, &DecodedRorM, DecodedInst);
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

    struct parsed_inst ParsedPop = {0};
    ParsedPop.Binary = DecodedInst->Binary;
    ParsedPop.Disp8 = *(s8 *)DecodedInst->Binary + 2;
    ParsedPop.Disp16 = *(s16 *)DecodedInst->Binary + 2;
    ParsedPop.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedPop.RorM = (ByteTwo & R_OR_M_FIELD);

    struct string DecodedReg = {0};
    DecodedReg.Ptr = DecodedReg.String;
    struct string DecodedRorM = {0};
    DecodedRorM.Ptr = DecodedRorM.String;
    struct string Operand = {0};
    Operand.Ptr = Operand.String;

    if( (ByteOne >= POP_SEG_REG) && (ByteOne < POP_REG) )
    {
	DecodedInst->Size = 1;
	ParsedPop.Reg = ((ByteOne & CHECK_SEG_REG) >> 3);
	SteenCopy(&Operand, SegRegLUT[ParsedPop.Reg], REGISTER_NAME_LEN);
    }

    else if( (ByteOne >= POP_REG) && (ByteOne < POP_REG_OR_MEM) )
    {
	DecodedInst->Size = 1;
	ParsedPop.Reg = (ByteOne & 0x07);
	char *WhichRegIsIt = DecodeReg(1, ParsedPop.Reg);
	SteenCopy(&Operand, WhichRegIsIt, REGISTER_NAME_LEN);
    }

    else if(ByteOne >= POP_REG_OR_MEM)
    {
	DecodedInst->Size = 2;

	SteenCopy(&Operand, "word ", 5); 
	if(ParsedPop.Mod == MEM_MODE_DISP8)
	{
	    DecodedInst->Size += 1;
	}
	else if(ParsedPop.Mod == MEM_MODE_DISP16)
	{
	    DecodedInst->Size += 2;
	}
	DecodeRorM(&ParsedPop, &DecodedRorM, DecodedInst);
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

    struct parsed_inst ParsedXchg = {0};
    ParsedXchg.Binary = DecodedInst->Binary;
    ParsedXchg.Disp8 = *(s8 *)DecodedInst->Binary + 2;
    ParsedXchg.Disp16 = *(s16 *)DecodedInst->Binary + 2;
    ParsedXchg.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedXchg.Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedXchg.RorM = (ByteTwo & R_OR_M_FIELD);
    ParsedXchg.IsWord = (bool)(ByteOne & 0x01);

    struct string DecodedReg = {0};
    DecodedReg.Ptr = DecodedReg.String;
    struct string DecodedRorM = {0};
    DecodedRorM.Ptr = DecodedRorM.String;

    if( (ByteOne >= XCHG_REG_or_MEM_WITH_REG) && (ByteOne < XCHG_REG_WITH_ACCUM) )
    {
	DecodedInst->Size = 2;

	char *WhichRegIsIt = DecodeReg(ParsedXchg.IsWord, ParsedXchg.Reg);
	SteenCopy(&DecodedReg, WhichRegIsIt, REGISTER_NAME_LEN);
	DecodeRorM(&ParsedXchg, &DecodedRorM, DecodedInst);

	if(ParsedXchg.Mod == MEM_MODE_DISP8)
	{
	    DecodedInst->Size += 1;
	}
	else if(ParsedXchg.Mod == MEM_MODE_DISP16)
	{
	    DecodedInst->Size += 2;
	}
	SteenCopy(&DecodedInst->OperandOne, DecodedRorM.String, DecodedRorM.Len);
	SteenCopy(&DecodedInst->OperandTwo, DecodedReg.String, DecodedReg.Len);

	1;
    }

    else if( (ByteOne >= XCHG_REG_WITH_ACCUM) && (ByteOne <= XCHG_UPPER_LIMIT) ) 
    {
	ParsedXchg.Reg = (ByteOne & 0x07);
	ParsedXchg.IsWord = true;

	if(DecodedInst->OpcodeEnum == NOP)
	{
	    DecodedInst->Mnemonic = "xchg";
	}
	DecodedInst-> Size = 1;

	// All Xchg instructions are Word size?
	SteenCopy(&DecodedInst->OperandOne, "ax", REGISTER_NAME_LEN);
	
	// we have to override the word flag this instruction doesn't use it
	//    and it can be set erroneously
	ParsedXchg.IsWord = 1;
	char *WhichRegIsIt = DecodeReg(ParsedXchg.IsWord, ParsedXchg.Reg);

	SteenCopy(&DecodedInst->OperandTwo, WhichRegIsIt, REGISTER_NAME_LEN);

	1;
    }

    else
    {

	Debug_PrintCurrentStatus(DecodedInst);
	Debug_OutputErrorMessage("Couldn't decode ByteOne of Xchg instruction", __func__, __LINE__);
	exit(1);
    }
}

internal void 
// stop: starting on add instruction
DecodeArithmetic(struct decoded_inst *DecodedInst)
{
    u8 ByteOne = DecodedInst->Binary[0];
    u8 ByteTwo = DecodedInst->Binary[1];

    struct parsed_inst ParsedArithmetic = {0};
    ParsedArithmetic.Binary = DecodedInst->Binary;

    // Check these offsets
    ParsedArithmetic.Disp8 = *(s8 *)DecodedInst->Binary + 2;
    ParsedArithmetic.Disp16 = *(s16 *)DecodedInst->Binary + 2;
    ParsedArithmetic.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedArithmetic.Reg = ((ByteTwo & FLEX_FIELD) >> 3);
    ParsedArithmetic.RorM = (ByteTwo & R_OR_M_FIELD);

    struct string DecodedReg = {0};
    DecodedReg.Ptr = DecodedReg.String;
    struct string DecodedRorM = {0};
    DecodedRorM.Ptr = DecodedRorM.String;
    
    /*// Check that we need this variable*/
    /*struct string Operand = {0};*/
    /*Operand.Ptr = Operand.String;*/

    if( (ByteOne >= ARITHMETIC_REGorMEM_WITH_REG_TO_EITHER) && (ByteOne < ARITHMETIC_IMM_TO_ACCUM) )
    {
	// DON'T FORGET
	DecodedInst->Size = 2;

	char *WhichRegIsIt = DecodeReg(ParsedArithmetic.IsWord, ParsedArithmetic.Reg);
	SteenCopy(&DecodedReg, WhichRegIsIt, REGISTER_NAME_LEN);

	DecodeRorM(&ParsedArithmetic, &DecodedRorM, DecodedInst);
	if(ParsedArithmetic.Mod == MEM_MODE_DISP8)
	{
	    DecodedInst->Size = 3;
	}
	else if( (ParsedArithmetic.Mod == MEM_MODE_DISP16) ||
		    (ParsedArithmetic.Mod == MEM_MODE_NO_DISP && ParsedArithmetic.RorM == DIRECT_ADDRESS) )
	{
	    DecodedInst->Size = 4;
	}
	if(ParsedArithmetic.DestFlag)
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

    else if( (ByteOne >= ARITHMETIC_IMM_TO_ACCUM) && (ByteOne < ARITHMETIC_IMM_TO_REGorMEM) )
    {
	char ImmediateValueBuffer[MAX_STRING_LEN];
	int NumValueChars = 0;

	// DON'T FORGET
	if(ParsedArithmetic.IsWord)
	{
	    DecodedInst->Size = 3;
	    SteenCopy(&DecodedInst->OperandOne, "ax", 2);

	    s16 ImmediateValue = *(s16 *)(ParsedArithmetic.Binary + 1);
	    NumValueChars = Get16BitsAsChars(ImmediateValueBuffer, ImmediateValue);
	    DecodedInst->Size = 3;
	}

	else
	{
	    DecodedInst->Size = 2;
	    SteenCopy(&DecodedInst->OperandOne, "al", 2);

	    s8 ImmediateValue = *(s8 *)(ParsedArithmetic.Binary + 1);
	    NumValueChars = Get8BitsAsChars(ImmediateValueBuffer, ImmediateValue);
	    DecodedInst->Size = 2;
	}

	SteenCopy(&DecodedInst->OperandTwo, ImmediateValueBuffer, NumValueChars);
    }

    else if(ByteOne >= ARITHMETIC_IMM_TO_REGorMEM)
    {
	DecodeRorM(&ParsedArithmetic, &DecodedRorM, DecodedInst);
	int NumDispBytes = 0;
	DecodedInst->Size = 3;

	if(ParsedArithmetic.Mod == MEM_MODE_DISP8)
	{
	    DecodedInst->Size += 1;
	    NumDispBytes = 1;
	}
	else if(ParsedArithmetic.Mod == MEM_MODE_DISP16)
	{
	    DecodedInst->Size += 2;
	    NumDispBytes = 2;
	}

	struct string ExplicitSize = {0};
	ExplicitSize.Ptr = ExplicitSize.String;
	char ImmediateValueBuffer[MAX_STRING_LEN];
	int NumImmediateChars = 0;

	if(ParsedArithmetic.IsWord)
	{
	    DecodedInst->Size += 1;
	    SteenCopy(&ExplicitSize, "word ", 5);
	    s16 ImmediateValue = *(s16 *)(ParsedArithmetic.Binary + NumDispBytes + 2);
	    NumImmediateChars = Get16BitsAsChars(ImmediateValueBuffer, ImmediateValue);
	    SteenCopy(&ExplicitSize, ImmediateValueBuffer, NumImmediateChars);
	}
	else
	{	    
	    SteenCopy(&ExplicitSize, "byte ", 5);
	    s8 ImmediateValue = *(s8 *)(ParsedArithmetic.Binary + NumDispBytes + 2);
	    NumImmediateChars = Get8BitsAsChars(ImmediateValueBuffer, ImmediateValue);
	    SteenCopy(&ExplicitSize, ImmediateValueBuffer, NumImmediateChars);
	}

	SteenCopy(&DecodedInst->OperandOne, DecodedRorM.String, DecodedRorM.Len);
	SteenCopy(&DecodedInst->OperandTwo, ExplicitSize.String, ExplicitSize.Len);
    }

    else
    {
	// Error
	Debug_PrintCurrentStatus(DecodedInst);
	Debug_OutputErrorMessage("Couldn't decode Arithmetic", __func__, __LINE__);
	
	exit(1);
    }
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
		    return(TBD_SHL);
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
		    exit(1);
		} break;
	    }
	} break;

	case 0xFE:
	{
	    switch(IHaveMetTheDistinguisher)
	    {
		case 0x00: // 000
		{
		    return(INC);
		} break;

		case 0x10: // 010
		{
		    return(DEC);
		} break;

		default:
		{
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
		    exit(1);
		} break;
	    }
	} break;

	default:
	{
	    exit(1);
	} break;
    }
}

internal void
Dispatch(struct decoded_inst *DecodedInst)
{
    if(DecodedInst->OpcodeEnum == MOV)
    {
	DecodeMov(DecodedInst);
    }
    else if(DecodedInst->OpcodeEnum == PUSH)
    {
	DecodePush(DecodedInst);
    }
    else if(DecodedInst->OpcodeEnum == POP)
    {
	DecodePop(DecodedInst);
    }
    else if(DecodedInst->OpcodeEnum == XCHG || DecodedInst->OpcodeEnum == NOP)
    {
	DecodeXchg(DecodedInst);
    }
    else if(DecodedInst->OpcodeEnum == ADD || DecodedInst->OpcodeEnum == SUB || DecodedInst->OpcodeEnum == CMP)
    {
	DecodeArithmetic(DecodedInst);
    }
    else
    {
	// Error
	Debug_PrintCurrentStatus(DecodedInst);
	Debug_OutputErrorMessage("No matching dispatch function", __func__, __LINE__);
	
	exit(1);
    }
}
