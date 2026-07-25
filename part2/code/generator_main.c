// Check if cluster or uniform
// Pick 5 random cluster centers
// For each, generate count / 5 pairs
// Write the pairs to JSON
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>

typedef double f64;
typedef uint32_t u32;
typedef uint64_t u64;

#define EARTH_RADIUS 6372.8f

#define Assert(Condition) if(!(Condition)) {*(volatile int *)0 = 1;}

struct pair
{
    f64 X0;
    f64 Y0;
    f64 X1;
    f64 Y1;
};

struct random_series
{
    u64 A, B, C, D;
};

#define Debug_OutputErrorMessage(Msg) __Debug_OutputErrorMessage(Msg, __func__, __LINE__)

void
__Debug_OutputErrorMessage(char *ErrorMessage, const char *CallingFunction, int Line)
{
    char ErrorBuffer[256];
    sprintf_s(ErrorBuffer, sizeof(ErrorBuffer), 
              "\nERROR:\n  In function %s, on line %d,\n\n    ", CallingFunction, Line);
    OutputDebugStringA(ErrorBuffer);
    sprintf_s(ErrorBuffer, sizeof(ErrorBuffer), ErrorMessage);
    OutputDebugStringA(ErrorBuffer);
    OutputDebugStringA(".\n\n");
}

FILE *
Open(u64 PairCount, char *Filename)
{
    char FilenameBuffer[256];
    sprintf(FilenameBuffer, "data_%llu_%s", PairCount, Filename);
    FILE *Result = fopen(FilenameBuffer, "wb");
    if(!Result)
    {
        Debug_OutputErrorMessage("Failed to open file");
        exit(1);
    }

    return(Result);
}

u64 
RotateLeft(u64 V, int Shift)
{

    // e.g., V = 1111 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001 
    //
    // Rotate left 7
    //
    // (V << Shift) = (V << 7) = 
    //          0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 1000 0000 
    //
    // (V >> (64 - Shift)) = (V >> (64 - 7)) = (V >> 57) = 
    //          0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0111 1000     
    //
    // (OR together)
    //          0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 1000 0000 
    //                                              OR
    //          0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0111 1000     
    //
    //        = 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 1111 1000     
    u64 Result = ((V << Shift) | (V >> (64 - Shift)));
    return(Result);
}

u64
RandomU64(struct random_series *Series)
{
    u64 A = Series->A;
    u64 B = Series->B;
    u64 C = Series->C;
    u64 D = Series->D;

    u64 E = A - RotateLeft(B, 27);

    A = (B ^ RotateLeft(C, 17));
    B = (C + D);
    C = (D + E);
    D = (E + A);

    Series->A = A;
    Series->B = B;
    Series->C = C;
    Series->D = D;

    return(D);
}

struct random_series
SeedRandomSeries(u64 Value)
{
    struct random_series Series = {};

    Series.A = 0xf1ea5eed;
    Series.B = Value;
    Series.C = Value;
    Series.D = Value;

    u32 Count = 20;
    while(Count--)
    {
        RandomU64(&Series);
    }

    return(Series);
}

f64
RandomInRange(struct random_series *Series, f64 Min, f64 Max)
{
    // A + t(B - A)
    // A + tB - tA
    // A - tA + tB
    // A(1 - t) + tB
    f64 t = (f64)RandomU64(Series) / (f64)UINT64_MAX;
    f64 Result = Min + t*(Max - Min);
    
    return(Result);
}

f64
RandomDegree(struct random_series *Series, f64 Center, f64 Radius, f64 MaxAllowed)
{
    f64 Min = Center - Radius;
    if(Min < -MaxAllowed)
    {
        Min = -MaxAllowed;
    }

    f64 Max = Center + Radius;
    if(Max > MaxAllowed)
    {
        Max = MaxAllowed;
    }

    f64 Result = RandomInRange(Series, Min, Max);
    return(Result);
}

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
Haversine(struct pair Pair)
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

// Generating a new cluster.
//      XCenter = RandomInRange(&Series, -MaxAllowedX, MaxAllowedX);
//      YCenter = RandomInRange(&Series, -MaxAllowedY, MaxAllowedY);
//              XCenter (longitude) is a value in range of -180, 180
//              YCenter (latitude) is a value in range of -90, 90
//
//      XRadius = RandomInRange(&Series, 0, MaxAllowedX);
//      YRadius = RandomInRange(&Series, 0, MaxAllowedY);
//              XRadius is a value in range of 0, 180
//              YRadius is a value in range of 0, 90.
//
//      RandomDegree(random_series *Series, f64 Center, f64 Radius, f64 MaxAllowed)              
//              f64 X0 = RandomDegree(&Series, XCenter, XRadius, MaxAllowedX);
//              f64 Y0 = RandomDegree(&Series, YCenter, YRadius, MaxAllowedY);
//
//
//              f64 MinVal = Center - Radius;
//              if(MinVal < -MaxAllowed)
//              {
//                  MinVal = -MaxAllowed;
//              }
//
//              f64 MaxVal = Center + Radius;
//              if(MaxVal > MaxAllowed)
//              {
//                  MaxVal = MaxAllowed;
//              }
//              
//
//      Example.
//          XRadius generated at 173.05430...
//          YRadius generated at 0.29683...
//          RandomDegree called with CenterX value of -36.70992...
//          
//          MinVal = Center - Radius:
//              -36.70992 - 173.05430 = -209.76423...
//              And that value is clamped to -180.
//
//          MaxVal = Center + Radius:
//              -36.70992 + 173.05430 = 136.344368...
//              No clamp needed.
//
//          Call RandomInRange:
//              f64 Result = RandomInRange(Series, MinVal, MaxVal)
//              Which lerps between Min and Max with a random value between 0 and 1.
//





int
main(int ArgCount, char **ArgumentVector)
{
    if(ArgCount != 4)
    {
        // ArgVector for Casey's generator goes: [executable name] [uniform or cluster] [seed] [number of coordinate pairs to generate]
        // Right now we just hardcode those values
        OutputDebugStringA("\n\nUsage: [executable name] [uniform or cluster] [seed] [number of coordinate pairs to generate]\n\n");
        exit(1);
    }

    char *Method = ArgumentVector[1];
    u64 ClusterCountLeft = UINT64_MAX;
    if(strcmp(Method, "cluster") == 0)
    {
        ClusterCountLeft = 0;
    }
    else if (strcmp(Method, "uniform") != 0)
    {
        OutputDebugStringA("\n\nMethod must be either cluster or uniform; defaulting to uniform\n\n");
    }

    u64 Seed = atoll(ArgumentVector[2]);
    u64 NumPairs = atoll(ArgumentVector[3]);

    struct random_series Series = SeedRandomSeries(Seed);

    f64 XCenter = 0;
    f64 YCenter = 0;
    f64 XRadius = 0;
    f64 YRadius = 0;

    // Longitude:
    //      Look at the earth from above one of the poles; the earth becomes a 2-D circle. 
    //      Two points have the same longitude when a line emanating from one of the poles
    //              (i.e., center of the circle) passes through both points. 
    //      A point's longitude is given by an angle measurement in the range of -180 to 180 degrees.
    //      
    // Latitude:
    //      Look at the earth such that your eyes are level with the equator; the earth becomes a 2-D circle.
    //      Draw horizontal lines (chords) across the circle.
    //      Two points have the same latitude when they lie along the same horizontal line.
    //      The equator has a latitude of 0 degrees.
    //      The south pole has a latitude of -90 degrees.
    //      The north pole has a latitude of 90 degrees.
    f64 MaxLongitude = 180;
    f64 MaxLatitude = 90;

    // Group the pairs of points into K = (2 + Floor(NumPoints / 64)) cluster groups.
    // i.e., Every Kth pair gets a new cluster group.
    u64 ClusterCountMax = 1 + (NumPairs / 64);
    
    f64 Accumulator = 0;

    FILE *JsonOutput = Open(NumPairs, "Output.json");
    FILE *ComputedHaversines = Open(NumPairs, "ComputedHaversines.f64");
    if(!(JsonOutput && ComputedHaversines))
    {
        Debug_OutputErrorMessage("One or both of JsonOutput and ComputedHaversines couldn't be opened");
        exit(1);
    }

    fprintf(JsonOutput, "{\"pairs\":[\n");

    for(int PairIdx = 0;
        PairIdx < NumPairs;
        ++PairIdx)
    {
        if(ClusterCountLeft-- == 0)
        {
            ClusterCountLeft = ClusterCountMax;
            XCenter = RandomInRange(&Series, -MaxLongitude, MaxLongitude);
            YCenter = RandomInRange(&Series, -MaxLatitude, MaxLatitude);
            XRadius = RandomInRange(&Series, 0, MaxLongitude);
            YRadius = RandomInRange(&Series, 0, MaxLatitude);
        }
        
        struct pair Pair;
        Pair.X0 = RandomDegree(&Series, XCenter, XRadius, MaxLongitude);
        Pair.Y0 = RandomDegree(&Series, YCenter, YRadius, MaxLatitude);
        Pair.X1 = RandomDegree(&Series, XCenter, XRadius, MaxLongitude);
        Pair.Y1 = RandomDegree(&Series, YCenter, YRadius, MaxLatitude);

        f64 HaversineDistance = Haversine(Pair);

        Accumulator += HaversineDistance;

        char *Separator = (PairIdx == (NumPairs - 1)) ? "\n" : ",\n";

        fprintf(JsonOutput, "{\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f}%s",
                                Pair.X0, Pair.Y0, Pair.X1, Pair.Y1, Separator);
        fwrite(&HaversineDistance, sizeof(HaversineDistance), 1, ComputedHaversines);
    }

    Accumulator /= NumPairs;

    fprintf(JsonOutput, "]}\n");
    fwrite(&Accumulator, sizeof(Accumulator), 1, ComputedHaversines);


    char PrintBuffer[256];
    sprintf(PrintBuffer, "Expected sum: %f\n", Accumulator);
    OutputDebugStringA(PrintBuffer);

    if(JsonOutput) fclose(JsonOutput);
    if(ComputedHaversines) fclose(ComputedHaversines);

    return(0);
}


