enum json_token_type
{
    Token_OpenBrace,
    Token_OpenBracket,
    Token_CloseBrace,
    Token_CloseBracket,
    Token_Comma,
    Token_Colon,
    Token_Semicolon,

    Token_StringLiteral,

    Token_True,
    Token_False,
    Token_Null,

    Token_Number
};


struct json_parser
{
    struct buffer JsonToParse;
    u64 Cursor;
    bool HadError;
};

struct json_token
{
    enum json_token_type Type;
    struct buffer Buffer;
};

struct json_element
{
    struct buffer KeyIfItExists;
    struct buffer Value;
    struct json_element *FirstSubElement;
    struct json_element *NextSibling;
};

bool
IsInBounds(struct buffer Buffer, u64 Cursor)
{
    bool Result = ((Buffer.Data + Cursor) < (Buffer.Data + Buffer.NumBytes));

    return(Result);
}

bool
IsJsonDigit(struct buffer Buffer, u64 Cursor)
{
    bool Result = false;
    if(IsInBounds(Buffer, Cursor))
    {
        u8 Value = Buffer.Data[Cursor];
        Result = ((Value >= '0') && (Value <= '9'));
    }

    return(Result);
}

bool 
IsJsonWhitespace(struct buffer Buffer, u64 Cursor)
{
    bool Result = false;
    if(IsInBounds(Buffer, Cursor))
    {
        u8 Value = Buffer.Data[Cursor];
        Result = ((Value == ' ') || (Value == '\t') || (Value == '\n') || (Value == '\r'));
    }

    return(Result);
}


struct json_token
GetJsonToken(struct json_parser *Parser)
{
    struct json_token Result = {0};
    struct buffer JsonToParse = Parser->JsonToParse;
    u64 LocalCursor = Parser->Cursor;

    Result.Buffer.NumBytes = 1;
    Result.Buffer.Data = Parser->JsonToParse.Data + LocalCursor;

    while(IsJsonWhitespace(JsonToParse, LocalCursor))
    {
        ++LocalCursor;
    }

    if(IsInBounds(JsonToParse, LocalCursor))
    {
        u8 NextChar = Parser->JsonToParse.Data[LocalCursor];

        switch(NextChar)
        {
            // 
            case '{': { Result.Type = Token_OpenBrace; goto IncrementLocalCursor; }
            case '[': { Result.Type = Token_OpenBracket;goto IncrementLocalCursor;}
            case '}': { Result.Type = Token_CloseBrace;goto IncrementLocalCursor;}
            case ']': { Result.Type = Token_CloseBracket;goto IncrementLocalCursor;}
            case ',': { Result.Type = Token_Comma;goto IncrementLocalCursor;}
            case ':': { Result.Type = Token_Colon;goto IncrementLocalCursor;}
            
            IncrementLocalCursor:
            if(IsInBounds(JsonToParse, LocalCursor+1))
            {
                ++LocalCursor;
            }
            break;

                      // I think semicolons are disallowed in JSON
                      // case ';': { Result.Type = Token_Semicolon; } break;

                      // String literals
            case '"':
                      {
                          Result.Type = Token_StringLiteral;

                          // Advance local cursor so that it points at the first character after the " character
                          ++LocalCursor;

                          //  This block advances LocalCursor until it points at the " character that terminates
                          //      the string.
                          //
                          //  e.g.,
                          //      "pairs"
                          //       012345
                          //
                          //       variable StringStart is set to value of LocalCursor, which is the 0th byte in the string.
                          //       LocalCursor advances until it points at the terminating ", skipping instances of
                          //              \" along the way.
                          //       When LocalCursor points to ", loop exits.
                          //
                          //       e.g., The length of pairs is 5.
                          //       LocalCursor points at the 5th byte (").
                          //       LocalCursor - StringStart = 5 - 0 = 5.
                          u64 StringStart = LocalCursor;
                          while(IsInBounds(JsonToParse, LocalCursor) && (JsonToParse.Data[LocalCursor] != '"'))
                          {
                              if((JsonToParse.Data[LocalCursor] == '\\') &&
                                 IsInBounds(JsonToParse, LocalCursor+1) &&
                                 (JsonToParse.Data[LocalCursor+1] == '"'))
                              {
                                  ++LocalCursor;
                              }
                              ++LocalCursor;
                          }

                          Result.Buffer.NumBytes = LocalCursor - StringStart;
                          Result.Buffer.Data = JsonToParse.Data + StringStart;
                          if(IsInBounds(JsonToParse, LocalCursor+1))
                          {
                              ++LocalCursor;
                          }
                      } break;

                      // Numbers
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                      {
                          Result.Type = Token_Number;

                          // Similar to above, we record where the number starts, and then we consume characters until we find where it ends.
                          u64 NumberStart = LocalCursor;

                          // We have to identify, and skip over, any of these characters that the number may contain:
                          //      Leading minus sign
                          //      Digits before the decimal point
                          //      The decimal point
                          //      Digits after the decimal point
                          //      Scientific notation characters:
                          //          The scientific notation indicators "e" or "E"
                          //          A plus or minus sign 
                          //          Digits

                          // Skip leading minus sign
                          if((JsonToParse.Data[LocalCursor] == '-') && IsInBounds(JsonToParse, LocalCursor+1))
                          {
                              ++LocalCursor;
                          }

                          // Skip digits before the decimal point
                          if(IsJsonDigit(JsonToParse, LocalCursor))
                          {
                              while(IsJsonDigit(JsonToParse, LocalCursor) && IsInBounds(JsonToParse, LocalCursor+1))
                              {
                                  ++LocalCursor;
                              }
                          }

                          // Find the decimal point if it exists
                          // Skip digits after it
                          if((JsonToParse.Data[LocalCursor] == '.') && IsInBounds(JsonToParse, LocalCursor+1))
                          {
                              ++LocalCursor;
                              while(IsJsonDigit(JsonToParse, LocalCursor) && IsInBounds(JsonToParse, LocalCursor+1))
                              {
                                  ++LocalCursor;
                              }
                          }

                          // Scientific notation
                          if(((JsonToParse.Data[LocalCursor] == 'e') || (JsonToParse.Data[LocalCursor] == 'E')) &&
                             IsInBounds(JsonToParse, LocalCursor+1))
                          {
                              ++LocalCursor;

                              if(((JsonToParse.Data[LocalCursor] == '+') || (JsonToParse.Data[LocalCursor] == '-')) &&
                                 IsInBounds(JsonToParse, LocalCursor+1))
                              {
                                  ++LocalCursor;
                              }

                              while(IsJsonDigit(JsonToParse, LocalCursor) && IsInBounds(JsonToParse, LocalCursor+1))
                              {
                                  ++LocalCursor;
                              }
                          }

                          Result.Buffer.NumBytes = LocalCursor - NumberStart;
                      } break;

        }

    }

    Parser->Cursor = LocalCursor;
    return(Result);
}

bool
ShouldKeepParsing(struct json_parser Parser)
{
    bool Result = (!Parser.HadError && IsInBounds(Parser.JsonToParse, Parser.Cursor));

    return(Result);
}

// Put signature here so we can call container parser in ParseJSONElement
struct json_element *
ParseJsonContainer(struct json_parser *Parser, struct json_token StartingToken, enum json_token_type EndingTokenShouldBe, bool IsKeyValue);

struct json_element *
ParseJsonElement(struct json_parser *Parser, struct buffer KeyIfItExists, struct json_token InitialToken)
{
    bool Valid = true;
    struct json_element *FirstSubElementForThisContainer = 0;

    if(InitialToken.Type == Token_OpenBrace)
    {
        FirstSubElementForThisContainer = ParseJsonContainer(Parser, InitialToken, Token_CloseBrace, true);
    }
    else if(InitialToken.Type == Token_OpenBracket)
    {
        FirstSubElementForThisContainer = ParseJsonContainer(Parser, InitialToken, Token_CloseBracket, false);
    }
    else if((InitialToken.Type == Token_StringLiteral) ||
            (InitialToken.Type == Token_True) ||
            (InitialToken.Type == Token_False) ||
            (InitialToken.Type == Token_Null) ||
            (InitialToken.Type == Token_Number))
    {
        // Just store this value. No further parsing required
    }
    else
    {
        Valid = false;
    }

    struct json_element *Result = {0};
    if(Valid)
    {
        Result = (struct json_element *)malloc(sizeof(struct json_element));
        Result->KeyIfItExists = KeyIfItExists;
        Result->Value = InitialToken.Buffer;
        Result->FirstSubElement = FirstSubElementForThisContainer;
        Result->NextSibling = 0;
    }

    return(Result);
}

struct json_element *
ParseJsonContainer(struct json_parser *Parser, struct json_token StartingToken, enum json_token_type EndingTokenShouldBe, bool IsKeyValue)
{
    struct json_element *FirstElementInContainer = {};
    struct json_element *LastElementInContainer = {};

    while(ShouldKeepParsing(*Parser))
    {
        // If this container is a key-value object (not a plain array), the only valid options for GetJsonToken to return are:
        //      1. A string literal that is the key for the next value
        //      2. A Token_CloseBrace that means the container is exhausted
        struct buffer KeyIfItExists = {};
        struct json_token NextToken = GetJsonToken(Parser);
        if(IsKeyValue)
        {
            if(NextToken.Type == Token_StringLiteral)
            {
                // GetJsonToken has returned a string literal that is the key in the next key-value pair in the container.
                KeyIfItExists = NextToken.Buffer;

                // By royal decree, the key must be followed by a colon
                struct json_token ShouldBeColon = GetJsonToken(Parser);
                if(ShouldBeColon.Type == Token_Colon)
                {
                    NextToken = GetJsonToken(Parser);
                }
                else
                {
                    // Setting this is redundant for now since we quit immediately afterward with our debug error function, 
                    //      but if we expand this in the future, we will want to set an error state and try to 
                    //      print out some info on the exact site of failure before exiting
                    Parser->HadError = true;

                    // Note: We cannot pass json_tokens to this error function because they are not null-terminated
                    //      and instead rely on the count field in the struct; in short, they are not compatible
                    //      with any libc string functions
                    OutputErrorMessage("Did not find expected colon after reading string literal that should \
                                            have been a key in JSON object");
                    exit(1);
                }
            }
            else if(NextToken.Type != EndingTokenShouldBe)
            {
                Parser->HadError = true;
                OutputErrorMessage("Did not get expected close brace when parsing K-V container");
                exit(1);
            }
        }
        
        struct json_element *NewElement = ParseJsonElement(Parser, KeyIfItExists, NextToken);
        if(NewElement)
        {
            // Build the linked list.
            //      If the list doesn't contain any nodes yet, LastElementInContainer will be null. Set both First 
            //          and LastElementInContainer to the malloc'd value returned by ParseJsonElement.
            //
            //      If the list already has at least one node, only set LastElementInContainer to the
            //          malloc'd value returned by ParseJsonElement.
            if(!LastElementInContainer)
            {
                FirstElementInContainer = NewElement;
                LastElementInContainer = NewElement;
            }
            else
            {
                LastElementInContainer->NextSibling = NewElement;
                LastElementInContainer = NewElement;
            }
        }
        else if(NextToken.Type == EndingTokenShouldBe)
        {
            break;
        }
        else
        {
            Parser->HadError = true;

            OutputErrorMessage("Unexpected token when parsing Json k-v container");
            exit(1);
        }

        struct json_token CommaOrEndingToken = GetJsonToken(Parser);
        if(CommaOrEndingToken.Type == EndingTokenShouldBe)
        {
            break;
        }
        else if(CommaOrEndingToken.Type != Token_Comma)
        {
            Parser->HadError = true;

            OutputErrorMessage("Unexpected token when parsing Json k-v container");
            exit(1);
        }
    }

    return(FirstElementInContainer);
}

bool
AreBuffersEqual(struct buffer A, struct buffer B)
{
    if(A.NumBytes != B.NumBytes)
    {
        return false;
    }

    for(u64 Cursor = 0;
        Cursor < A.NumBytes;
        ++Cursor)
    {
        if(A.Data[Cursor] != B.Data[Cursor])
        {
            return false;
        }
    }

    return(true);
}

struct json_element *
LookUpJsonElement(struct json_element *Object, struct buffer Key)
{
    struct json_element *Result = 0;

    if(Object)
    {
        for(struct json_element *Rover = Object->FirstSubElement; 
            Rover; 
            Rover = Rover->NextSibling)
        {
            if(AreBuffersEqual(Key, Rover->KeyIfItExists))
            {
                Result = Rover;
                break;
            }
        }
    }

    return(Result);
}

struct json_element *
ParseJson(struct json *Json)
{
    struct json_parser Parser;
    Parser.JsonToParse = Json->JsonToParse;
    Parser.HadError = false;
    Parser.Cursor = 0;

    struct buffer NeedEmptyBufferToGetThisGoing = {};
    struct json_element *Result = ParseJsonElement(&Parser, NeedEmptyBufferToGetThisGoing, GetJsonToken(&Parser));
    return(Result);
}

f64
ConvertJsonSign(struct buffer Source, u64 *CursorInJsonString)
{
    u64 LocalCursor = *CursorInJsonString;

    f64 Result = 1.0;
    if(IsInBounds(Source, LocalCursor) && (Source.Data[LocalCursor] == '-'))
    {
        Result = -1.0;
        if(IsInBounds(Source, LocalCursor+1))
        {
            ++LocalCursor;
        }
    }

    *CursorInJsonString = LocalCursor;

    return(Result);
}

f64
ConvertJsonNumber(struct buffer Source, u64 *CursorInJsonString)
{
    u64 LocalCursor = *CursorInJsonString;

    f64 Result = 0;
    while(IsInBounds(Source, LocalCursor))
    {
        u8 ConvertToBinary = Source.Data[LocalCursor] - (u8)'0';
        if(ConvertToBinary < 10)
        {
            Result = 10.0f*Result + (f64)ConvertToBinary;
            ++LocalCursor;
        }
        else
        {
            break;
        }
    }

    *CursorInJsonString = LocalCursor;

    return(Result);
}

f64
ConvertJsonStringToF64(struct json_element *JsonObject, struct buffer Key)
{
    f64 Result = 0;

    struct json_element *Element = LookUpJsonElement(JsonObject, Key);
    if(Element)
    {
        struct buffer Source = Element->Value;
        u64 LocalCursor = 0;

        f64 Sign = ConvertJsonSign(Source, &LocalCursor);

        // Just get the whole number part of the float first
        f64 Number = ConvertJsonNumber(Source, &LocalCursor);

        // Now get the fractional part
        if(IsInBounds(Source, LocalCursor) && (Source.Data[LocalCursor] == '.'))
        {
            ++LocalCursor;
            f64 Coefficient = 1.0f / 10.0f;
            while(IsInBounds(Source, LocalCursor))
            {
                u8 ConvertToBinary = Source.Data[LocalCursor] - (u8)'0';
                if(ConvertToBinary < 10)
                {
                    Number = Number + Coefficient* (f64)ConvertToBinary;
                    Coefficient *= 1.0f / 10.0f;
                    ++LocalCursor;
                }
                else
                {
                    break;
                }
            }
        }

        // Handle scientific notation if it exists
        // First skip e or E and a leading + if it exists
        if(IsInBounds(Source, LocalCursor) && 
           ((Source.Data[LocalCursor] == 'e') || (Source.Data[LocalCursor] == 'E')))
        {
            ++LocalCursor;
            if(IsInBounds(Source, LocalCursor) && (Source.Data[LocalCursor] == '+'))
            {
                ++LocalCursor;
            }

            // In the case that there was a minus sign for the expontent, the above check for '+'
            //      would have been skipped, and we'd be pointing at it now; ConvertJsonSign
            //      would properly return -1.0f.
            //
            //  In the case that there was no sign, we'd be pointing at a number here. But that's 
            //      OK, because in the case that the value at LocalCursor is not a '-', ConvertJsonNumber
            //      just returns 1.0f, and does not advance the cursor. 
            //      So the call itself and the multiply below with ExponentSign as an operand have no effect.
            f64 ExponentSign = ConvertJsonSign(Source, &LocalCursor);
            f64 Exponent = ExponentSign*ConvertJsonNumber(Source, &LocalCursor);
            Number *= pow(10.0f, Exponent);
        }

        Result = Sign * Number;
    }

    return(Result);
}


// struct json_element
// {
//     struct buffer KeyIfItExists;
//     struct buffer Value;
//     struct json_element *FirstSubElement;
//     struct json_element *NextSibling;
// };
// 

void
FreeJson(struct json_element *Node)
{
    while(Node)
    {
        struct json_element *FreeNode = Node;
        Node = FreeNode->NextSibling;

        FreeJson(FreeNode->FirstSubElement);
        free(FreeNode);
    }
}



// Define FreeNext as Element->NextSibling
// Replace
