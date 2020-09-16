/*
The Math namespace contains implementations for 4-vectors and 4x4 matrices, which are used to describe rotation and translation in 3d space.
For the purpose of remaining lightweight and fast, implementations of M34.mul and M34.invert are without loops or arrays, which can cause significant overhead.
The result of this is ugly and tedious code, but the sacrifice of cleanliness for speed is a necessary one.
*/
#pragma once

namespace Math {
    struct V3
    {
        float x, y, z;

        V3();
        V3(float x, float y, float z);
        void print();
    };

    struct M34
    {
        V3 x, y, z, t;

        M34(V3 x, V3 y, V3 z, V3 t);
        // matrix multiplication: (4x4) x (4x1) -> (4x1)
        V3 mul(V3 p);
        // matrix inversion: (4x4).invert() x (4x4) -> (4x4) identity
        M34 invert();
        void print();
    };

    struct M3
    {
        float
        xx, yx, zx,
        xy, yy, zy,
        xz, yz, zz;

        void print();
    };

    // get a 3x3 matrix to rotate a vector angle degrees around the u axis
    M3 getRotationMatrix(V3 u, float angle);

    V3 matMul3(M3 m, V3 x);
}
