#include "Math.h"
#include "3DG.h"
#include "RenderEngine.h"
#include "lodepng.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <fstream>
#define _USE_MATH_DEFINES
#include <cmath>

const float Z_MAX = 4096;
const float eps = .01;

using Math::V3, DG::Camera, DG::SV, DG::STriangle, DG::Image, DG::Material, DG::Triangle, std::min, std::max;

void getVertex(std::string s, int &v, int &vt)
{
    int slashIndex = s.find('/');
    v = std::stoi(s.substr(0, slashIndex)) - 1;
    s = s.substr(slashIndex + 1);

    if (slashIndex != -1)
    {
        slashIndex = s.find('/');
        if (!s.substr(0, slashIndex).empty())
        {
            vt = std::stoi(s.substr(0, slashIndex));
        }
        else
        {
            vt = 0;
        }
    }
    else
    {
        vt = 0;
    }
}

int findIndexOfMaterial(std::string name, std::vector<Material> &ms)
{
    for (int i = 0; i < ms.size(); i++)
    {
        if (ms[i].name == name) { return i; }
    }
    return -1;
}

Object::Object(std::string filename): filename(filename) {}

// end me
bool Object::loadObj()
{
    std::ifstream objFile;
    objFile.open(filename + ".obj");
    if (!objFile.good()) { return false; }

    std::vector<V3> vs;
    std::vector<float> vts = {0, 0};
    std::vector<Face> fs;

    std::string line;

    std::ifstream mtlFile;
    mtlFile.open(filename + ".mtl");
    ms.push_back(Material());

    if (mtlFile.good())
    {
        while (getline(mtlFile, line))
        {
            if (!line.compare(0, 7, "newmtl "))
            {
                Material m = Material();
                m.name = line.substr(7);

                getline(mtlFile, line);
                m.Ns = std::stof(line.substr(3));

                getline(mtlFile, line);
                line = line.substr(3);
                std::stringstream ssA(line);
                std::string val;
                ssA >> val;
                m.Ka.x = std::stof(val);
                ssA >> val;
                m.Ka.y = std::stof(val);
                ssA >> val;
                m.Ka.z = std::stof(val);

                getline(mtlFile, line);
                line = line.substr(3);
                std::stringstream ssD(line);
                ssD >> val;
                m.Kd.x = std::stof(val);
                ssD >> val;
                m.Kd.y = std::stof(val);
                ssD >> val;
                m.Kd.z = std::stof(val);

                getline(mtlFile, line);
                getline(mtlFile, line);
                getline(mtlFile, line);
                getline(mtlFile, line);

                getline(mtlFile, line);
                line = line.substr(6);
                m.illum = (!line.compare(0, 1, "2")) ? true : false;

                getline(mtlFile, line);
                if (!line.empty())
                {
                    std::string filename = line.substr(7);
                    int error = lodepng::decode(m.texture.pixels, m.texture.width, m.texture.height, filename);
                    if (error)
                    {
                        printf("Could not load texture \"%s\" for object! LodePNG Error: %s\n", filename.c_str(), lodepng_error_text(error));
                        return false;
                    }
                    m.hasTexture = true;
                }
                ms.push_back(m);
            }
        }
    }

    std::string current_material;
    while (getline(objFile, line))
    {
        if (!line.compare(0, 2, "v "))
        {
            // push the vertex coordinates to "vs" vector
            line = line.substr(2);
            std::stringstream ss(line);
            std::string val;
            V3 v = V3(0, 0, 0);
            ss >> val;
            v.x = std::stof(val);
            ss >> val;
            v.y = std::stof(val);
            ss >> val;
            v.z = std::stof(val);
            vs.push_back(v);
        }
        if (!line.compare(0, 3, "vt "))
        {
            // push the vertex coordinates to "vs" vector
            line = line.substr(3);
            std::stringstream ss(line);
            std::string val;
            ss >> val;
            vts.push_back(std::stof(val));
            ss >> val;
            vts.push_back(std::stof(val));
        }
        if (!line.compare(0, 7, "usemtl "))
        {
            current_material = line.substr(7);
        }
        if (!line.compare(0, 2, "f "))
        {
            // push the face vertexes to "fs" vector
            line = line.substr(2);
            Face f;
            f.material = current_material;
            int i = 0;
            int spaceIndex = 0;
            while (i < 4 && spaceIndex != -1)
            {
                spaceIndex = line.find(' ');
                getVertex(line.substr(0, spaceIndex), f.v[i], f.vt[i]);
                line = line.substr(spaceIndex + 1);
                i++;
            }

            if (i == 3)
            {
                f.threeVertices = true;
            }
            fs.push_back(f);
        }
    }

    // get the vertexes that are described by the faces and create triangles
    for (int i = 0; i < fs.size(); i++)
    {
        int mIndex = findIndexOfMaterial(fs[i].material, ms);
        Material* m;
        if (mIndex == -1)
        { m = &ms[0]; }
        else
        { m = &ms[mIndex]; }

        ts.push_back(Triangle(vs[fs[i].v[0]], vs[fs[i].v[1]], vs[fs[i].v[2]], m, vts[2 * fs[i].vt[0]], vts[2 * fs[i].vt[0] + 1], vts[2 * fs[i].vt[1]], vts[2 * fs[i].vt[1] + 1], vts[2 * fs[i].vt[2]], vts[2 * fs[i].vt[2] + 1]));
        // if the face has 4 vertices instead of 3, push another triangle
        if (!fs[i].threeVertices)
        {
            ts.push_back(Triangle(vs[fs[i].v[0]], vs[fs[i].v[2]], vs[fs[i].v[3]], m, vts[2 * fs[i].vt[0]], vts[2 * fs[i].vt[0] + 1], vts[2 * fs[i].vt[2]], vts[2 * fs[i].vt[2] + 1], vts[2 * fs[i].vt[3]], vts[2 * fs[i].vt[3] + 1]));
        }
    }
    return true;
}

bool inView(const SV &v)
{
    return v.shouldRender;
}

inline float edge(const int &x0, const int &y0, const int &x1, const int &y1, const int &x2, const int &y2)
{
    return (x1 - x0) * (y2 - y0) + (y0 - y1) * (x2 - x0);
}

bool RenderEngine::init(std::string windowName, int _screenX, int _screenY, float fov)
{
    screenX = _screenX;
    screenY = _screenY;
    if (SDL_Init(SDL_INIT_EVERYTHING))
    {
        return false;
    }

    window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenX, screenY, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    windowSurface = SDL_GetWindowSurface(window);
    if (windowSurface == NULL)
    {
        printf("Window surface could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    pixels = (Uint32*) windowSurface->pixels;

    c = Camera(screenX, screenY, fov);
    zBuffer = new float[screenX * screenY];

    return true;
}

RenderEngine::~RenderEngine()
{
    delete[] zBuffer;

    SDL_FreeSurface(windowSurface);
    SDL_DestroyWindow(window);

    windowSurface = NULL;
    window = NULL;

    SDL_Quit();
}

void RenderEngine::newFrame()
{
    // reset the z buffer and set the screen to black
    std::fill_n(zBuffer, screenX * screenY, Z_MAX);
    // std::fill_n(pixels, screenX * screenY, 0xFFFFFFFF);
    for (int j = 0; j < screenY; j++)
    {
        float t = 1 - (float) j / screenY;
        Uint8 r = 255 * (1 - .5 * t);
        Uint8 g = 255 * (1 - .3 * t);
        Uint8 b = 255;
        for (int i = 0; i < screenX; i++)
        {
            pixels[j * screenX + i] = SDL_MapRGBA(windowSurface->format, r, g, b, 0xFF);
        }
    }
    // start counting mspf
    prevTime = SDL_GetTicks();
    // get the position of the light
    lLight = c.worldToLocal(light);
}

void RenderEngine::endFrame()
{
    SDL_UpdateWindowSurface(window);
    mspf = SDL_GetTicks() - prevTime;
}

void RenderEngine::printMSPF()
{
    printf("MSPF: %f\n", mspf);
}

void RenderEngine::hideCursor()
{
    SDL_ShowCursor(SDL_DISABLE);
}

void RenderEngine::showCursor()
{
    SDL_ShowCursor(SDL_ENABLE);
}

void RenderEngine::resetMouse()
{
    SDL_WarpMouseInWindow(window, screenX / 2, screenY / 2);
}

void RenderEngine::renderObj(Object &obj)
{
    for (int i = 0; i < obj.ts.size(); i++)
    {
        V3 n;
        Triangle l = obj.ts[i].worldToLocal(c, n);
        // backface culling
        // if the normal vector faces towards the camera, render the triangle
        if (l.v0.x * n.x + l.v0.y * n.y + l.v0.z * n.z < 0)
        {
            renderTriangle(l.localToScreen(c), l, light, n);
        }
    }
}

void RenderEngine::renderObjSmooth(Object &obj)
{
    for (int i = 0; i < obj.ts.size(); i++)
    {
        V3 n;
        Triangle l = obj.ts[i].worldToLocal(c, n);
        // backface culling
        // if the normal vector faces towards the camera, render the triangle
        if (l.v0.x * n.x + l.v0.y * n.y + l.v0.z * n.z < 0)
        {
            if (l.m->hasTexture)
            { renderTriangleTexture(l.localToScreen(c), l, light, n); }
            else
            { renderTriangleSmooth(l.localToScreen(c), l, light, n); }
        }
    }
}

void RenderEngine::renderTriangle(const STriangle &t, const Triangle &l, const V3 &light, const V3 &n)
{
    if (inView(t.v0) && inView(t.v1) && inView(t.v2))
    {
        // get the bounding box of the triangle and the area of the triangle in screen space
        int xmin = max(0, min(screenX, min(min(t.v0.x, t.v1.x), t.v2.x)));
        int ymin = max(0, min(screenY, min(min(t.v0.y, t.v1.y), t.v2.y)));
        int xmax = max(0, min(screenX, max(max(t.v0.x, t.v1.x), t.v2.x)));
        int ymax = max(0, min(screenY, max(max(t.v0.y, t.v1.y), t.v2.y)));
        float area = edge(t.v2.x, t.v2.y, t.v0.x, t.v0.y, t.v1.x, t.v1.y);

        float x = (l.v0.x + l.v1.x + l.v2.x) / 3;
        float y = (l.v0.y + l.v1.y + l.v2.y) / 3;
        float z = (l.v0.z + l.v1.z + l.v2.z) / 3;
        float rX = light.x - x;
        float rY = light.y - y;
        float rZ = light.z - z;
        float length = pow(rX * rX + rY * rY + rZ * rZ, .5);
        rX /= length;
        rY /= length;
        rZ /= length;
        float brightness = max(0.0f, rX * n.x + rY * n.y + rZ * n.z);
        float spec = 0;
        if (l.m->illum)
        {
            length = pow(x * x + y * y + z * z, .5);
            x /= length;
            y /= length;
            z /= length;
            rX -= x;
            rY -= y;
            rZ -= z;
            length = pow(rX * rX + rY * rY + rZ * rZ, .5);
            rX /= length;
            rY /= length;
            rZ /= length;
            spec = max(0.0f, rX * n.x + rY * n.y + rZ * n.z);
            spec = pow(spec, l.m->Ns);
        }
        float r = Uint8 (min(255.0f, 255.0f * (brightness * l.m->Kd.x + spec)));
        float g = Uint8 (min(255.0f, 255.0f * (brightness * l.m->Kd.y + spec)));
        float b = Uint8 (min(255.0f, 255.0f * (brightness * l.m->Kd.z + spec)));

        for (int i = xmin; i < xmax; i++)
        {
            for (int j = ymin; j < ymax; j++)
            {
                // get the barycentric coefficients to determine if the pixel should be colored
                // and get the distance to the pixel for the z buffer
                float e0 = edge(i, j, t.v1.x, t.v1.y, t.v2.x, t.v2.y) / area;
                float e1 = edge(i, j, t.v2.x, t.v2.y, t.v0.x, t.v0.y) / area;
                float e2 = 1 - e0 - e1;
                // eps removes artifacts
                if (e0 >= -eps && e1 >= -eps && e2 >= -eps)
                {
                    int index = j * screenX + i;
                    z = 1 / (e0 / l.v0.z + e1 / l.v1.z + e2 / l.v2.z);
                    if (z < zBuffer[index])
                    {
                        zBuffer[index] = z;
                        pixels[index] = SDL_MapRGBA(windowSurface->format, r, g, b, 0xFF);
                    }
                }
            }
        }
    }
}

void RenderEngine::renderTriangleSmooth(const STriangle &t, const Triangle &l, const V3 &light, const V3 &n)
{
    if (inView(t.v0) && inView(t.v1) && inView(t.v2))
    {
        // get the bounding box of the triangle and the area of the triangle in screen space
        int xmin = max(0, min(screenX, min(min(t.v0.x, t.v1.x), t.v2.x)));
        int ymin = max(0, min(screenY, min(min(t.v0.y, t.v1.y), t.v2.y)));
        int xmax = max(0, min(screenX, max(max(t.v0.x, t.v1.x), t.v2.x)));
        int ymax = max(0, min(screenY, max(max(t.v0.y, t.v1.y), t.v2.y)));
        float area = edge(t.v2.x, t.v2.y, t.v0.x, t.v0.y, t.v1.x, t.v1.y);

        for (int i = xmin; i < xmax; i++)
        {
            for (int j = ymin; j < ymax; j++)
            {
                // get the barycentric coefficients to determine if the pixel should be colored
                // and get the distance to the pixel for the z buffer
                float e0 = edge(i, j, t.v1.x, t.v1.y, t.v2.x, t.v2.y) / area;
                float e1 = edge(i, j, t.v2.x, t.v2.y, t.v0.x, t.v0.y) / area;
                float e2 = 1 - e0 - e1;
                // eps removes artifacts
                if (e0 >= -eps && e1 >= -eps && e2 >= -eps)
                {
                    int index = j * screenX + i;
                    float z = 1 / (e0 / l.v0.z + e1 / l.v1.z + e2 / l.v2.z);
                    if (z < zBuffer[index])
                    {
                        zBuffer[index] = z;

                        float x = z * (l.v0.x / l.v0.z * e0 + l.v1.x / l.v1.z * e1 + l.v2.x / l.v2.z * e2);
                        float y = z * (l.v0.y / l.v0.z * e0 + l.v1.y / l.v1.z * e1 + l.v2.y / l.v2.z * e2);
                        float rX = light.x - x;
                        float rY = light.y - y;
                        float rZ = light.z - z;
                        float length = pow(rX * rX + rY * rY + rZ * rZ, .5);
                        rX /= length;
                        rY /= length;
                        rZ /= length;
                        float brightness = max(0.0f, rX * n.x + rY * n.y + rZ * n.z);
                        float spec = 0;
                        if (l.m->illum)
                        {
                            length = pow(x * x + y * y + z * z, .5);
                            x /= length;
                            y /= length;
                            z /= length;
                            rX -= x;
                            rY -= y;
                            rZ -= z;
                            length = pow(rX * rX + rY * rY + rZ * rZ, .5);
                            rX /= length;
                            rY /= length;
                            rZ /= length;
                            spec = max(0.0f, rX * n.x + rY * n.y + rZ * n.z);
                            spec = pow(spec, l.m->Ns);
                        }
                        pixels[index] = SDL_MapRGBA(windowSurface->format,
                            Uint8 (min(255.0f, 255.0f * (brightness * l.m->Kd.x + spec))),
                            Uint8 (min(255.0f, 255.0f * (brightness * l.m->Kd.y + spec))),
                            Uint8 (min(255.0f, 255.0f * (brightness * l.m->Kd.z + spec))),
                            0xFF);
                    }
                }
            }
        }
    }
}

void RenderEngine::renderTriangleTexture(const STriangle &t, const Triangle &l, const V3 &light, const V3 &n)
{
    if (inView(t.v0) && inView(t.v1) && inView(t.v2))
    {
        // get the bounding box of the triangle and the area of the triangle in screen space
        int xmin = max(0, min(screenX, min(min(t.v0.x, t.v1.x), t.v2.x)));
        int ymin = max(0, min(screenY, min(min(t.v0.y, t.v1.y), t.v2.y)));
        int xmax = max(0, min(screenX, max(max(t.v0.x, t.v1.x), t.v2.x)));
        int ymax = max(0, min(screenY, max(max(t.v0.y, t.v1.y), t.v2.y)));
        float area = edge(t.v2.x, t.v2.y, t.v0.x, t.v0.y, t.v1.x, t.v1.y);

        for (int i = xmin; i < xmax; i++)
        {
            for (int j = ymin; j < ymax; j++)
            {
                // get the barycentric coefficients to determine if the pixel should be colored
                // and get the distance to the pixel for the z buffer
                float e0 = edge(i, j, t.v1.x, t.v1.y, t.v2.x, t.v2.y) / area;
                float e1 = edge(i, j, t.v2.x, t.v2.y, t.v0.x, t.v0.y) / area;
                float e2 = 1 - e0 - e1;
                // eps removes artifacts
                if (e0 >= -eps && e1 >= -eps && e2 >= -eps)
                {
                    e0 = max(0.0f, e0);
                    e1 = max(0.0f, e1);
                    e2 = max(0.0f, e2);

                    int index = j * screenX + i;
                    float z = 1 / (e0 / l.v0.z + e1 / l.v1.z + e2 / l.v2.z);
                    if (z < zBuffer[index])
                    {
                        zBuffer[index] = z;

                        float x = z * (l.v0.x / l.v0.z * e0 + l.v1.x / l.v1.z * e1 + l.v2.x / l.v2.z * e2);
                        float y = z * (l.v0.y / l.v0.z * e0 + l.v1.y / l.v1.z * e1 + l.v2.y / l.v2.z * e2);
                        float u = z * (l.v0u / l.v0.z * e0 + l.v1u / l.v1.z * e1 + l.v2u / l.v2.z * e2);
                        float v = z * (l.v0v / l.v0.z * e0 + l.v1v / l.v1.z * e1 + l.v2v / l.v2.z * e2);
                        float rX = light.x - x;
                        float rY = light.y - y;
                        float rZ = light.z - z;
                        float length = pow(rX * rX + rY * rY + rZ * rZ, .5);
                        rX /= length;
                        rY /= length;
                        rZ /= length;
                        float brightness = max(0.0f, rX * n.x + rY * n.y + rZ * n.z);
                        float spec = 0;
                        if (l.m->illum)
                        {
                            length = pow(x * x + y * y + z * z, .5);
                            x /= length;
                            y /= length;
                            z /= length;
                            rX -= x;
                            rY -= y;
                            rZ -= z;
                            length = pow(rX * rX + rY * rY + rZ * rZ, .5);
                            rX /= length;
                            rY /= length;
                            rZ /= length;
                            spec = max(0.0f, rX * n.x + rY * n.y + rZ * n.z);
                            spec = 255 * pow(spec, l.m->Ns);
                        }


                        Uint8 r, g, b;
                        l.m->texture.pixel(fmod(fmod(u, 1) + 1, 1), fmod(fmod(v, 1) + 1, 1), r, g, b);
                        pixels[index] = SDL_MapRGBA(windowSurface->format,
                            Uint8 (min(255.0f, (brightness * r + spec))),
                            Uint8 (min(255.0f, (brightness * g + spec))),
                            Uint8 (min(255.0f, (brightness * b + spec))),
                            0xFF);
                    }
                }
            }
        }
    }
}
