#if !defined(ASTEROIDS_MATH_H)

union v2
{
    struct
    {
        f32 x, y;
    };
    
    
    f32 E[2];
};

union v2_i
{
    struct
    {
        s32 x, y;
    };
    s32 E[2];
};

union v3
{
    struct
    {
        f32 x, y, z;
    };
    struct
    {
        v2 xy;
        f32 _y;
    };
    struct
    {
        f32 _x;
        v2 yx;
    };
    struct
    {
        f32 r, g, b;
    };
    f32 E[3];
};

union v4
{
    struct
    {
        f32 x, y, z, w;
    };
    union
    {
        struct
        {
            v3 rgb;
            f32 a;
        };
        struct
        {
            f32 r, g, b, a;
        };
    };
    f32 E[4];
};

v2 V2(f32 x, f32 y)
{
    v2 Result;
    
    Result.x = x;
    Result.y = y;
    
    return Result;
}

v3 V3(f32 x, f32 y, f32 z)
{
    v3 Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    
    return Result;
}

v4 V4(f32 x, f32 y, f32 z, f32 w)
{
    v4 Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    Result.w = w;
    
    return Result;
}

v2_i V2_i(u32 x, u32 y)
{
    v2_i Result;
    
    Result.x = x;
    Result.y = y;
    
    return Result;
}

v2 V2i(s32 x, s32 y)
{
    v2 Result;
    
    Result.x = (f32)x;
    Result.y = (f32)y;
    
    return Result;
}

v2 V2i(u32 x, u32 y)
{
    v2 Result;
    
    Result.x = (f32)x;
    Result.y = (f32)y;
    
    return Result;
}

v3 V3i(u32 x, u32 y, u32 z)
{
    v3 Result;
    
    Result.x = (f32)x;
    Result.y = (f32)y;
    Result.z = (f32)z;
    
    return Result;
}

v3 V3i(s32 x, s32 y, s32 z)
{
    v3 Result;
    
    Result.x = (f32)x;
    Result.y = (f32)y;
    Result.z = (f32)z;
    
    return Result;
}

v4 V4i(u32 x, u32 y, u32 z, u32 w)
{
    v4 Result;
    
    Result.x = (f32)x;
    Result.y = (f32)y;
    Result.z = (f32)z;
    Result.w = (f32)w;
    
    return Result;
}

v4 V4i(s32 x, s32 y, s32 z, s32 w)
{
    v4 Result;
    
    Result.x = (f32)x;
    Result.y = (f32)y;
    Result.z = (f32)z;
    Result.w = (f32)w;
    
    return Result;
}

v2
operator-(v2 A)
{
    v2 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    
    return Result;
}

v2
operator-(v2 A, f32 B)
{
    v2 Result;
    
    Result.x = A.x - B;
    Result.y = A.y - B;
    
    return Result;
}

v2
operator-(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return Result;
}

v2 &
operator-=(v2 &A, v2 B)
{
    A = A - B;
    
    return A;
}

v2
operator+(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return Result;
}

v2
operator+(v2 A, f32 B)
{
    v2 Result;
    
    Result.x = A.x + B;
    Result.y = A.y + B;
    
    return Result;
}

v2 &
operator+=(v2 &A, v2 B)
{
    A = A + B;
    
    return A;
}

v2 &
operator+=(v2 &A, f32 B)
{
    A = A + B;
    
    return A;
}

v2
operator*(v2 A, f32 Real32)
{
    v2 Result;
    
    Result.x = A.x * Real32;
    Result.y = A.y * Real32;
    
    return Result;
}

v2
operator*(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    
    return Result;
}

v2
operator*(f32 Real32, v2 A)
{
    v2 Result = A * Real32;
    
    return Result;
}



v2 &
operator*=(v2 &A, f32 Real32)
{
    A = A * Real32;
    
    return A;
}

v2 &
operator*=(f32 Real32, v2 &A)
{
    A = A * Real32;
    
    return A;
}

v2 &
operator*=(v2 &A, v2 B)
{
    A = A * B;
    
    return A;
}

v2
operator/(v2 A, f32 Real32)
{
    v2 Result;
    
    Result.x = A.x / Real32;
    Result.y = A.y / Real32;
    
    return Result;
}

v2
operator/(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    
    return Result;
}

v2 &
operator/=(v2 &A, f32 Real32)
{
    
    A = A / Real32;
    
    return A;
}

v2 &
operator/=(v2 &A, v2 B)
{
    
    A = A / B;
    
    return A;
}

// // // V3 // // // 

v3
operator-(v3 A)
{
    v3 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    
    return Result;
}

v3
operator-(v3 A, f32 B)
{
    v3 Result;
    
    Result.x = A.x - B;
    Result.y = A.y - B;
    Result.z = A.z - B;
    
    return Result;
}

v3
operator-(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    
    return Result;
}

v3 &
operator-=(v3 &A, v3 B)
{
    A = A - B;
    
    return A;
}

v3
operator+(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return Result;
}

v3 &
operator+=(v3 &A, v3 B)
{
    A = A + B;
    
    return A;
}

v3
operator*(v3 A, f32 Real32)
{
    v3 Result;
    
    Result.x = A.x * Real32;
    Result.y = A.y * Real32;
    Result.z = A.z * Real32;
    
    return Result;
}

v3
operator*(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    Result.z = A.z * B.z;
    
    return Result;
}

v3
operator*(f32 Real32, v3 A)
{
    v3 Result = A * Real32;
    
    return Result;
}



v3 &
operator*=(v3 &A, f32 Real32)
{
    A = A * Real32;
    
    return A;
}

v3 &
operator*=(v3 &A, v3 B)
{
    A = A * B;
    
    return A;
}

v3 &
operator*=(f32 B, v3 &A)
{
    A = A * B;
    
    return A;
}

v3
operator/(v3 A, f32 Real32)
{
    v3 Result;
    
    Result.x = A.x / Real32;
    Result.y = A.y / Real32;
    Result.z = A.z / Real32;
    
    return Result;
}

v3
operator/(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    Result.z = A.z / B.z;
    
    return Result;
}

v3 &
operator/=(v3 &A, f32 Real32)
{
    
    A = A / Real32;
    
    return A;
}

v3 &
operator/=(v3 &A, v3 B)
{
    
    A = A / B;
    
    return A;
}

// // // V4 // // // 

v4
operator-(v4 A)
{
    v4 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    Result.w = -A.w;
    
    return Result;
}

v4
operator-(v4 A, f32 B)
{
    v4 Result;
    
    Result.x = A.x - B;
    Result.y = A.y - B;
    Result.z = A.z - B;
    Result.w = A.w - B;
    
    return Result;
}

v4
operator-(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    Result.w = A.w - B.w;
    
    return Result;
}

v4 &
operator-=(v4 &A, v4 B)
{
    A = A - B;
    
    return A;
}

v4
operator+(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;
    
    return Result;
}

v4 &
operator+=(v4 &A, v4 B)
{
    A = A + B;
    
    return A;
}

v4
operator*(v4 A, f32 Real32)
{
    v4 Result;
    
    Result.x = A.x * Real32;
    Result.y = A.y * Real32;
    Result.z = A.z * Real32;
    Result.w = A.w * Real32;
    
    return Result;
}

v4
operator*(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    Result.z = A.z * B.z;
    Result.w = A.w * B.w;
    
    return Result;
}

v4
operator*(f32 Real32, v4 A)
{
    v4 Result = A * Real32;
    
    return Result;
}



v4 &
operator*=(v4 &A, f32 Real32)
{
    A = A * Real32;
    
    return A;
}

v4 &
operator*=(v4 &A, v4 B)
{
    A = A * B;
    
    return A;
}

v4
operator/(v4 A, f32 Real32)
{
    v4 Result;
    
    Result.x = A.x / Real32;
    Result.y = A.y / Real32;
    Result.z = A.z / Real32;
    Result.w = A.w / Real32;
    
    return Result;
}

v4
operator/(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    Result.z = A.z / B.z;
    Result.w = A.w / B.w;
    
    return Result;
}

v4 &
operator/=(v4 &A, f32 Real32)
{
    
    A = A / Real32;
    
    return A;
}

v4 &
operator/=(v4 &A, v4 B)
{
    
    A = A / B;
    
    return A;
}

struct rect2
{
    v2 MinCorner;
    v2 MaxCorner;
};

// ######## FUNCTIONS #########


v2 Perp(v2 Vector)
{
    v2 Result = V2(-Vector.y, Vector.x);
    
    return Result;
}

f32 Sin(f32 Real32)
{
    f32 Result = sinf(Real32);
    
    return Result;
}

f32 Cos(f32 Real32)
{
    f32 Result = cosf(Real32);
    
    return Result;
}

u32 
RoundReal32ToUInt32(f32 Real32)
{
    u32 Result = (s32)roundf(Real32);
    
    return Result;
}

s32 
FloorReal32ToInt32(f32 Real32)
{
    s32 Result = (s32)floorf(Real32);
    
    return Result;
}

s32 
CeilReal32ToInt32(f32 Real32)
{
    s32 Result = (s32)ceilf(Real32);
    
    return Result;
}

f32
Square(f32 A)
{
    f32 Result;
    
    Result = A * A;
    
    return Result;
}

v2
Square(v2 A)
{
    v2 Result;
    
    Result.x = A.x * A.x;
    Result.y = A.y * A.y;
    
    return Result;
}

b32
IsPointInsideRect(v2 Point, rect2 TestRect)
{
    b32 Result = ((Point.x > TestRect.MinCorner.x) &&
                  (Point.y > TestRect.MinCorner.y) &&
                  (Point.x < TestRect.MaxCorner.x) &&
                  (Point.y < TestRect.MaxCorner.y));
    
    return Result;
}

rect2
ExpandBorders(rect2 Rect, v2 Dimension)
{
    rect2 Result;
    
    Result.MinCorner = V2(Rect.MinCorner.x - Dimension.x, Rect.MinCorner.y - Dimension.y);
    Result.MaxCorner = V2(Rect.MaxCorner.x + Dimension.x, Rect.MaxCorner.y + Dimension.y);
    
    return Result;
}

rect2
ExpandHalfDimBorders(rect2 Rect, v2 Dimension)
{
    rect2 Result;
    
    Dimension *= 0.5f;
    Result = ExpandBorders(Rect, Dimension);
    
    return Result;
}

rect2
Rect2(v2 Point, v2 Dimension)
{
    rect2 Result;
    
    Result.MinCorner = V2(Point.x - Dimension.x, Point.y - Dimension.y);
    Result.MaxCorner = V2(Point.x + Dimension.x, Point.y + Dimension.y);
    
    return Result;
}

rect2
HalfDimRect2(v2 Point, v2 Dimension)
{
    rect2 Result;
    
    Dimension *= 0.5f;
    Result = Rect2(Point, Dimension);
    
    return Result;
}

f32 
SquareRoot(f32 Real32)
{
    f32 Result = sqrtf(Real32);
    
    return Result;
}

f32 
Inner(v2 A, v2 B)
{
    f32 Result = A.x*B.x + A.y*B.y;
    
    return Result;
}


f32 
LengthSq(v2 Vector)
{
    f32 Result = Inner(Vector, Vector);
    
    return Result;
}

f32 
Length(v2 Vector)
{
    f32 Result = SquareRoot(LengthSq(Vector));
    
    return Result;
}

f32
Lerp(f32 A, f32 t, f32 B)
{
    f32 Result = (1.0f - t)*A + t*B;
    
    return Result;
}

v2
Lerp(v2 A, f32 t, v2 B)
{
    v2  Result = (1.0f - t)*A + t*B;
    
    return Result;
}

v4
Lerp(v4 A, f32 t, v4 B)
{
    v4  Result = (1.0f - t)*A + t*B;
    
    return Result;
}

v4 
SRGB255ToLinear1(v4 V)
{
    v4 Result;
    
    f32 Inv255 = 1.0f / 255.0f;
    
    Result.r = Square(Inv255*V.r);
    Result.g = Square(Inv255*V.g);
    Result.b = Square(Inv255*V.b);
    Result.a = Inv255*V.a;
    
    return Result;
}

v4
Linear1ToSRGB255(v4 V)
{
    v4 Result;
    
    f32 One255 = 255.0f;
    
    Result.r = One255*SquareRoot(V.r);
    Result.g = One255*SquareRoot(V.g);
    Result.b = One255*SquareRoot(V.b);
    Result.a = One255*V.a;
    
    return Result;
}

v2
MaximumV2(v2 A, v2 B)
{
    v2 Result;
    
    v2 Test = A - B;
    
    Result = ((Test.x <= 0) && (Test.y <= 0)) ? B : A;
    
    return Result;
}

v2
MinimumV2(v2 A, v2 B)
{
    v2 Result;
    
    v2 Test = A - B;
    
    Result = ((Test.x <= 0) && (Test.y <= 0)) ? A : B;
    
    return Result;
}

f32 Absolute(f32 Real32)
{
    f32 Result = (f32)fabs(Real32);
    
    return Result;
}

v2 Normalize(v2 V)
{
    f32 InvMul = 1.0f / Length(V);
    v2 Result = V*InvMul;
    
    return Result;
}

b32 V2_iEquals(v2_i A, v2_i B)
{
    b32 Result = true;
    
    Result = A.x == B.x ? Result : false;
    Result = A.y == B.y ? Result : false;
    
    return Result;
}

#define ASTEROIDS_MATH_H
#endif

