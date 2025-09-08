// DECODE FUNCTION TEMPLATE

internal void 
DecodeXXX(struct decoded_inst *DecodedInst)
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

    // Check these offsets
    ParsedInst.Disp8 = *(s8 *)DecodedInst->Binary + 2;
    ParsedInst.Disp16 = *(s16 *)DecodedInst->Binary + 2;
    ParsedInst.Mod = ((ByteTwo & MOD_FIELD) >> 6);
    ParsedInst.RorM = (ByteTwo & R_OR_M_FIELD);

    struct string DecodedReg = {0};
    DecodedReg.Ptr = DecodedReg.String;
    struct string DecodedRorM = {0};
    DecodedRorM.Ptr = DecodedRorM.String;
    
    // Check that we need this variable
    struct string Operand = {0};
    Operand.Ptr = Operand.String;

    if( (ByteOne >= XXX_XXX) && (ByteOne < XXX_XXX) )
    {
	// DON'T FORGET
	DecodedInst->Size = 1;

	// Check that common fields e.g. Reg aren't in unexpected places
	ParsedInst.Reg = ((ByteOne & CHECK_SEG_REG) >> 3);

	// Per above check that we're copying into a separate operand variable
	SteenCopy(&Operand, SegRegLUT[ParsedInst.Reg], REGISTER_NAME_LEN);

	1;
    }

    else if( (ByteOne >= XXX_XXX) && (ByteOne < XXX_XXX) )
    {
	// DON'T FORGET
	DecodedInst->Size = 1;
	ParsedInst.Reg = (ByteOne & 0x03);
	char *WhichRegIsIt = DecodeReg(1, ParsedInst.Reg);

	// Check
	SteenCopy(&Operand, WhichRegIsIt, REGISTER_NAME_LEN);

	1;
    }

    else if(ByteOne >= XXX_XXX)
    {
	// DON'T FORGET
	DecodedInst->Size = 2;

	// Check
	SteenCopy(&Operand, "word ", 5); 
	if(ParsedInst.Mod == MEM_MODE_DISP8)
	{
	    // DON'T FORGET
	    DecodedInst->Size += 1;
	}
	else if(ParsedInst.Mod == MEM_MODE_DISP16)
	{
	    // DON'T FORGET
	    DecodedInst->Size += 2;
	}
	DecodeRorM(&ParsedInst, &DecodedRorM, DecodedInst);
	SteenCopy(&Operand, DecodedRorM.String, DecodedRorM.Len);

	1;
    }
    else
    {
	// Error
	Debug_PrintCurrentStatus(DecodedInst);
	Debug_OutputErrorMessage("Couldn't decode XXX", __func__, __LINE__);
	
	exit(1);
    }
	// Check: This style
	SteenCopy(&DecodedInst->OperandOne, Operand.String, Operand.Len);

	// or this style
	SteenCopy(&DecodedInst->OperandOne, DecodedReg.String, DecodedReg.Len);
	SteenCopy(&DecodedInst->OperandTwo, ImmediateValueBuffer, NumValueChars);

	1;
}


// ERROR BLOCK

	Debug_PrintCurrentStatus(DecodedInst);
	Debug_OutputErrorMessage("No matching dispatch function", __func__, __LINE__);
	
	exit(1);
