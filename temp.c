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
                            DecodedInst->OpcodeEnum = ADD;
                            DecodedInst->DecodeGroup = GROUP;

                        } break;

                    case 0x08: // 001
                        {
                            DecodedInst->OpcodeEnum = OR;
                        } break;

                    case 0x10: // 010
                        {
                            DecodedInst->OpcodeEnum = ADC;
                        } break;

                    case 0x18: // 011
                        {
                            DecodedInst->OpcodeEnum = SBB;
                        } break;

                    case 0x20: // 100
                        {
                            DecodedInst->OpcodeEnum = AND;
                        } break;

                    case 0x28: // 101
                        {
                            DecodedInst->OpcodeEnum = SUB;
                        } break;

                    case 0x30: // 110
                        {
                            DecodedInst->OpcodeEnum = XOR;
                        } break;

                    case 0x38: // 111
                        {
                            DecodedInst->OpcodeEnum = CMP;
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
                            DecodedInst->OpcodeEnum = ADD;
                        } break;

                    case 0x10: // 010
                        {
                            DecodedInst->OpcodeEnum = ADC;
                        } break;

                    case 0x18: // 011
                        {
                            DecodedInst->OpcodeEnum = SBB;
                        } break;

                    case 0x28: // 101
                        {
                            DecodedInst->OpcodeEnum = SUB;
                        } break;

                    case 0x38: // 111
                        {
                            DecodedInst->OpcodeEnum = CMP;
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
                            DecodedInst->OpcodeEnum = ADD;
                        } break;

                    case 0x10: // 010
                        {
                            DecodedInst->OpcodeEnum = ADC;
                        } break;

                    case 0x18: // 011
                        {
                            DecodedInst->OpcodeEnum = SBB;
                        } break;

                    case 0x28: // 101
                        {
                            DecodedInst->OpcodeEnum = SUB;
                        } break;

                    case 0x38: // 111
                        {
                            DecodedInst->OpcodeEnum = CMP;
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
                            DecodedInst->OpcodeEnum = ROL;
                        } break;

                    case 0x08: // 001
                        {
                            DecodedInst->OpcodeEnum = ROR;
                        } break;

                    case 0x10: // 010
                        {
                            DecodedInst->OpcodeEnum = RCL;
                        } break;

                    case 0x18: // 011
                        {
                            DecodedInst->OpcodeEnum = RCR;
                        } break;

                    case 0x20: // 100
                        {
                            DecodedInst->OpcodeEnum = SHL;
                        } break;

                    case 0x28: // 101
                        {
                            DecodedInst->OpcodeEnum = SHR;
                        } break;

                    case 0x38: // 111
                        {
                            DecodedInst->OpcodeEnum = SAR;
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
                            DecodedInst->OpcodeEnum = TEST;
                        } break;

                    case 0x10: // 010
                        {
                            DecodedInst->OpcodeEnum = NOT;
                        } break;

                    case 0x18: // 011
                        {
                            DecodedInst->OpcodeEnum = NEG;
                        } break;

                    case 0x20: // 100
                        {
                            DecodedInst->OpcodeEnum = MUL;
                        } break;

                    case 0x28: // 101
                        {
                            DecodedInst->OpcodeEnum = IMUL;
                        } break;

                    case 0x30: // 110
                        {
                            DecodedInst->OpcodeEnum = DIV;
                        } break;

                    case 0x38: // 111
                        {
                            DecodedInst->OpcodeEnum = IDIV;
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
                            DecodedInst->OpcodeEnum = INC;
                        } break;

                    case 0x08: // 010
                        {
                            DecodedInst->OpcodeEnum = DEC;
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
                            DecodedInst->OpcodeEnum = INC;
                        } break;

                    case 0x08: // 001
                        {
                            DecodedInst->OpcodeEnum = DEC;
                        } break;

                    case 0x10: // 010
                    case 0x18: // 011
                        {
                            DecodedInst->OpcodeEnum = CALL;
                        } break;

                    case 0x20: // 100
                    case 0x28: // 101
                        {
                            DecodedInst->OpcodeEnum = JMP;
                        } break;

                    case 0x30: // 110
                        {
                            DecodedInst->OpcodeEnum = PUSH;
                        } break;

                    default:
                        {
                            Debug_OutputErrorMessage("Failed to decode extended opcode 0xFF:\n   INC, DEC, CALL, JMP or PUSH"); 
                            exit(1);
                        } break;
                }
            } break;

