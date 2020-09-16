#pragma once
#include "Math.h"
#include "3DG.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <vector>
#include <string>

using Math::V3, DG::Camera, DG::SV, DG::STriangle, DG::Image, DG::Material, DG::Triangle;

// helper class for loadObj
struct Face
{
    int v[4] = {0, 0, 0, 0};
    int vt[4] = {0, 0, 0, 0};
    bool threeVertices = false;
    std::string material = "";
};

// helper function for loadObj
int getVertex(std::string s);

struct Object
{
    std::vector<Triangle> ts;
    std::vector<Material> ms;
    std::string filename;
    Object(std::string filename);
    bool loadObj();
};

// checks if a screen vertex is in the view of the screen
bool inView(const SV &v);

// edge function, is positive if <x0, y0> is on the right of the ray from <x1, y1> to <x2, y2>
inline float edge(const int &x0, const int &y0, const int &x1, const int &y1, const int &x2, const int &y2);

class RenderEngine
{
private:
    SDL_Window* window = NULL;
    SDL_Surface* windowSurface = NULL;
    Uint32* pixels;
    float* zBuffer;
    int screenX, screenY;
    V3 light = V3(0, 0, 0);
    V3 lLight;
    // milliseconds per frame
    Uint32 prevTime;

public:
    float mspf;
    Camera c;
    bool init(std::string windowName, int screenX, int screenY, float fov);
    ~RenderEngine();
    // called at the start of a new frame,
    // clears zBuffer, pixels, and starts counting mspf
    void newFrame();
    // renders pixels and calculates mspf
    void endFrame();
    void printMSPF();
    // SDL mouse functions
    void hideCursor();
    void showCursor();
    void resetMouse();
    // renders an Object that consists of triangles, lighting is renderTriangle lighting
    void renderObj(Object &obj);
    // renders an Object that consists of triangles, lighting is renderTriangleSmooth lighting
    void renderObjSmooth(Object &obj);
    // renders a triangle with per triangle lighting, used in renderObj
    void renderTriangle(const STriangle &t, const Triangle &l, const V3 &light, const V3 &n);
    // renders a triangle with smooth lighting, used in renderObjSmooth
    void renderTriangleSmooth(const STriangle &t, const Triangle &l, const V3 &light, const V3 &n);
    // renders a triangle with a texture, used in renderObjSmooth
    void renderTriangleTexture(const STriangle &t, const Triangle &l, const V3 &light, const V3 &n);
};
