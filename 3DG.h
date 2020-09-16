#pragma once
#include "Math.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <string>
#include <vector>

using Math::V3, Math::M34;

namespace DG {
    struct SV
    {
        int x, y;
        bool shouldRender;
        SV();
        SV(int x, int y, bool shouldRender);
        void print();
    };

    class Camera
    {
    private:
        M34 position = M34(
            V3(1, 0, 0),
            V3(0, 1, 0),
            V3(0, 0, 1),
            V3(0, 0, 0)
            );
        M34 invertedPosition = position;
        int screenX;
        int screenY;
        float fov;

    public:
        Camera();
        Camera(int screenX, int screenY, float fov);
        // invert position and store in invertedPosition, to be called after move and rotate
        void invert();
        // translate camera, modifies position
        void move(float dx, float dy, float dz);
        void moveGM(float dx, float dy, float dz);
        void rotateX(float angle);
        void rotateXGM(float angle);
        void rotateY(float angle);
        void setFov(float f);
        float getFov();
        // transforms vector from world basis to camera basis
        V3 worldToLocal(V3 p);
        // transforms vector from camera basis to world basis
        V3 localToWorld(V3 p);
        // transforms (4x1) vector in camera basis to (2x1) vector in screen coordinates
        SV localToScreen(V3 l);
        void resetPosition();
    };

    struct STriangle
    {
        SV v0, v1, v2;

        STriangle() {}
        STriangle(SV v0, SV v1, SV v2);
    };

    struct Image
    {
        std::vector<Uint8> pixels;
        Uint32 width = 0;
        Uint32 height = 0;

        Image() {}
        Image(std::vector<Uint8> pixels, Uint32 width, Uint32 height);
        void pixel(float u, float v, Uint8 &r, Uint8 &g, Uint8 &b);
    };

    struct Material
    {
        std::string name = "";
        float Ns = 100;
        V3 Ka = V3(0, 0, 0);
        V3 Kd = V3(0.681, 0.479, .1);
        bool illum = true;
        bool hasTexture = false;
        Image texture;

        Material() {}
        void print();
    };

    struct Triangle
    {
        float v0u = 0, v0v = 0, v1u = 0, v1v = 0, v2u = 0, v2v = 0;
        Material* m;
        V3 v0, v1, v2;

        Triangle() {}
        Triangle(V3 v0, V3 v1, V3 v2, Material* m, float v0u, float v0v, float v1u, float v1v, float v2u, float v2v);
        Triangle worldToLocal(Camera &c, V3 &n);
        STriangle localToScreen(Camera &c);
        V3 normal();
    };
}
