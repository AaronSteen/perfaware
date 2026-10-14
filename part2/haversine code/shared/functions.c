f64
Square(f64 A)
{
    f64 Result = (A*A);
    return(Result);
}

f64
RadiansFromDegrees(f64 Degrees)
{
    f64 Result = 0.01745329251994329577 * Degrees;
    return(Result);
}

f64
Haversine(struct haversine_pair Pair)
{
    f64 Lat1 = Pair.Y0;
    f64 Lat2 = Pair.Y1;
    f64 Lon1 = Pair.X0;
    f64 Lon2 = Pair.X1;

    f64 dLat = RadiansFromDegrees(Lat2 - Lat1);
    f64 dLon = RadiansFromDegrees(Lon2 - Lon1);
    Lat1 = RadiansFromDegrees(Lat1);
    Lat2 = RadiansFromDegrees(Lat2);

    f64 A = Square(sin(dLat/2.0f)) + cos(Lat1) * cos(Lat2) * Square(sin(dLon/2.0f));
    f64 C = 2.0f * asin(sqrt(A));

    f64 Result = EARTH_RADIUS * C;

    return(Result);
}


#define OutputErrorMessage(...) \
    OutputErrorMessage_(__func__, __LINE__, __VA_ARGS__)

void
OutputErrorMessage_(const char *CallingFunction, int Line, const char *Format, ...)
{
    char Buffer[512];
    int Size = sizeof(Buffer);

    // snprintf returns the number of bytes it successfully printed
    int Offset = snprintf(Buffer, Size,
                          "\nERROR:\n    In function %s, on line %d,\n\n    ",
                          CallingFunction, Line);

    // We use the number of bytes snprintf successfully printed to determine
    //      how many bytes are left in Buffer
    if(Offset < 0)
    {
        // snprint returns negative if it failed to print anything.
        //      this function needs to always work, so let's exit here if this happens and fix it.
        OutputDebugStringA("\n\nFailed to write function and line info inside OutputErrorMessage_\n\n");
        exit(1);
    }
    else if(Offset > Size-1)
    {
        // If snprintf somehow filled the buffer, set the next available byte in the buffer to the last
        //      byte. This will cause vsnprintf below to fail to print anything and thereby not overflow the buffer.
        Offset = Size-1;
    }

    va_list Args;
    va_start(Args, Format);
    int Written = vsnprintf(Buffer + Offset, Size - Offset, Format, Args);
    va_end(Args);

    if(Written > 0)
    {
        Offset += Written;
    }
    if(Offset > Size-1)
    {
        Offset = Size-1;
    }

    snprintf(Buffer+Offset, Size-Offset, ".\n\n");

    OutputDebugStringA(Buffer);
}

char *
GetFileExtension(char *Filename)
{
    char *Extension = Filename;
    while(*Extension != '\0')
    {
        ++Extension;
    }
    while(*Extension != '.')
    {
        --Extension;
        if(Extension < Filename)
        {
            OutputErrorMessage("Supplied input file did not contain a . character. This utility only works with .json files");
            exit(1);
        }
    }
    return(Extension);
}

void
OpenJson(struct json *Json)
{
    Json->FilePointer = fopen(Json->Filename, "rb");
    if(!Json->FilePointer)
    {
        OutputErrorMessage("Failed to open file %s", Json->Filename);
        exit(1);
    }

    int SeekResult = _fseeki64(Json->FilePointer, 0, SEEK_END);
    if(SeekResult != 0)
    {
        OutputErrorMessage("Failed to seek to end of file %s", Json->Filename);
        exit(1);
    }

    s64 GetSizeResult = _ftelli64(Json->FilePointer);
    if(GetSizeResult == -1LL) 
    {
        // -1LL is result on fail
        OutputErrorMessage("Couldn't get size for file %s", Json->Filename);
        exit(1);
    }
    Json->JsonToParse.NumBytes = (size_t)GetSizeResult;

    _fseeki64(Json->FilePointer, 0, SEEK_SET);
}

struct buffer
AllocateBuffer(size_t NumBytes)
{
    struct buffer Result;
    Result.Data = (u8 *)malloc(NumBytes);
    if(Result.Data)
    {
        Result.NumBytes = NumBytes;
        return(Result);
    }
    else
    {
        OutputErrorMessage("Malloc failed");
        exit(1);
    }
}

static void
DebugOutput(const char *Format, ...)
{
    char PrintBuffer[512];
    int PrintBufferSize = sizeof(PrintBuffer);

    va_list Args;
    va_start(Args, Format);
    int BytesWritten = vsnprintf(PrintBuffer, PrintBufferSize, Format, Args);
    va_end(Args);

    if(BytesWritten > PrintBufferSize)
    {
        OutputDebugStringA("\n\nERROR: DebugOutput function format string did not fit in print buffer\n\n");
    }
    OutputDebugStringA(PrintBuffer);
    OutputDebugStringA("\n");

    return;
}
