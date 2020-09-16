#include "Math.h"
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>

using Math::V3, Math::M34, Math::M3;

V3::V3(): x(0), y(0), z(0) {}

V3::V3(float x, float y, float z): x(x), y(y), z(z) {}

void V3::print()
{
    printf("V3(%f, %f, %f)\n", x, y, z);
}


M34::M34(V3 x, V3 y, V3 z, V3 t): x(x), y(y), z(z), t(t) {}

V3 M34::mul(V3 p)
{
    return V3(
        x.x * p.x + y.x * p.y + z.x * p.z + t.x,
        x.y * p.x + y.y * p.y + z.y * p.z + t.y,
        x.z * p.x + y.z * p.y + z.z * p.z + t.z
        );
}

M34 M34::invert()
{
    return M34(
        V3(x.x, y.x, z.x),
        V3(x.y, y.y, z.y),
        V3(x.z, y.z, z.z),
        V3(
            -x.x * t.x - x.y * t.y - x.z * t.z,
            -y.x * t.x - y.y * t.y - y.z * t.z,
            -z.x * t.x - z.y * t.y - z.z * t.z)
        );
}

void M34::print()
{
    printf("M34(\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n)\n", x.x, y.x, z.x, t.x, x.y, y.y, z.y, t.y, x.z, y.z, z.z, t.z, 0.0f, 0.0f, 0.0f, 1.0f);
}

M3 Math::getRotationMatrix(V3 u, float angle)
{
    M3 r;
    float cosA = cos(angle);
    float oneMinusCosA = 1 - cosA;
    float sinA = sin(angle);
    float xSinA = u.x * sinA;
    float ySinA = u.y * sinA;
    float zSinA = u.z * sinA;
    float xy = u.x * u.y;
    float yz = u.y * u.z;
    float xz = u.x * u.z;
    r.xx = cosA + u.x * u.x * oneMinusCosA;
    r.yx = xy * oneMinusCosA - zSinA;
    r.zx = xz * oneMinusCosA + ySinA;
    r.xy = xy * oneMinusCosA + zSinA;
    r.yy = cosA + u.y * u.y * oneMinusCosA;
    r.zy = yz * oneMinusCosA - xSinA;
    r.xz = xz * oneMinusCosA - ySinA;
    r.yz = yz * oneMinusCosA + xSinA;
    r.zz = cosA + u.z * u.z * oneMinusCosA;
    return r;
}

V3 Math::matMul3(M3 m, V3 a)
{
    return V3(
        m.xx * a.x + m.yx * a.y + m.zx * a.z,
        m.xy * a.x + m.yy * a.y + m.zy * a.z,
        m.xz * a.x + m.yz * a.y + m.zz * a.z
        );
}

void M3::print()
{
    printf("M3(\n%f, %f, %f\n%f, %f, %f\n%f, %f, %f\n)\n", xx, yx, zx, xy, yy, zy, xz, yz, zz);
}
