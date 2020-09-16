#include "Math.h"
#include "3DG.h"
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>

using Math::V3, Math::M34, Math::M3, Math::getRotationMatrix, Math::matMul3,
DG::SV, DG::STriangle, DG::Image, DG::Material, DG::Triangle, DG::Camera;

SV::SV(int x, int y, bool shouldRender_): x(x), y(y)
{
    shouldRender = shouldRender_;
}

SV::SV(): x(0), y(0), shouldRender(false) {}

void SV::print()
{
    printf("SV<%i, %i, %s>\n", x, y, shouldRender ? "true" : "false");
}

Camera::Camera(): screenX(0), screenY(0)
{
    setFov(0);
}

Camera::Camera(int screenX, int screenY, float f): screenX(screenX), screenY(screenY)
{
    setFov(f);
}

void Camera::invert()
{
    invertedPosition = position.invert();
}

void Camera::move(float dx, float dy, float dz)
{
    position.t.x += position.x.x * dx + position.y.x * dy + position.z.x * dz;
    position.t.y += position.x.y * dx + position.y.y * dy + position.z.y * dz;
    position.t.z += position.x.z * dx + position.y.z * dy + position.z.z * dz;
}

void Camera::moveGM(float dx, float dy, float dz)
{
    float zLength = pow(position.z.x * position.z.x + position.z.z * position.z.z, .5);
    float xLength = pow(position.x.x * position.x.x + position.x.z * position.x.z, .5);
    position.t.x += position.z.x * dz / zLength + position.x.x / xLength * dx;
    position.t.z += position.z.z * dz / zLength + position.x.z / xLength * dx;
    position.t.y += dy;
}

void Camera::setFov(float f)
{
    fov = tan(f * M_PI / 360);
}

float Camera::getFov()
{
    return fov;
}

void Camera::rotateX(float angle)
{
    angle = angle * M_PI / 180;
    M3 r = getRotationMatrix(position.y, angle);
    position.x = matMul3(r, position.x);
    position.z = matMul3(r, position.z);
}

void Camera::rotateXGM(float angle)
{
    angle = angle * M_PI / 180;
    M3 r = getRotationMatrix(V3(0, 1, 0), angle);
    position.x = matMul3(r, position.x);
    position.y = matMul3(r, position.y);
    position.z = matMul3(r, position.z);
}

void Camera::rotateY(float angle)
{
    angle = angle * M_PI / 180;
    M3 r = getRotationMatrix(position.x, angle);
    position.y = matMul3(r, position.y);
    position.z = matMul3(r, position.z);
}

V3 Camera::worldToLocal(V3 p)
{
    return invertedPosition.mul(p);
}

V3 Camera::localToWorld(V3 p)
{
    return position.mul(p);
}

SV Camera::localToScreen(V3 l)
{
    return SV(ceil(screenX / 2 * (l.x / l.z / fov / screenX * screenY + 1)), ceil(screenY - screenY / 2 * (l.y / l.z / fov + 1)), l.z > 0 && l.z != 0);
}

void Camera::resetPosition()
{
    position = M34(
        V3(1, 0, 0),
        V3(0, 1, 0),
        V3(0, 0, 1),
        V3(0, 0, -2)
        );
    invert();
}

STriangle::STriangle(SV v0, SV v1, SV v2): v0(v0), v1(v1), v2(v2) {}

Image::Image(std::vector<Uint8> pixels, Uint32 width, Uint32 height): pixels(pixels), width(width), height(height) {}

void Image::pixel(float u, float v, Uint8 &r, Uint8 &g, Uint8 &b)
{
    int i = height - v * height;
    int j = width * u;
    int index = 4 * (i * width + j);
    r = pixels[index];
    g = pixels[++index];
    b = pixels[++index];
}

void Material::print()
{
    printf("Material %s\nNs %f\nKa ", name.c_str(), Ns);
    Ka.print();
    printf("Kd ");
    Kd.print();
    printf("illum %s\n", illum ? "true" : "false");
    if (hasTexture)
    {
        printf("Texture %zu, %zu\n", texture.width, texture.height);
    }
}

Triangle::Triangle(V3 v0, V3 v1, V3 v2, Material* m, float v0u, float v0v, float v1u, float v1v, float v2u, float v2v): v0(v0), v1(v1), v2(v2), m(m),
v0u(v0u), v0v(v0v), v1u(v1u), v1v(v1v), v2u(v2u), v2v(v2v) {}

Triangle Triangle::worldToLocal(Camera &c, V3 &n)
{
    Triangle l = Triangle(c.worldToLocal(v0), c.worldToLocal(v1), c.worldToLocal(v2), m, v0u, v0v, v1u, v1v, v2u, v2v);
    n = l.normal();
    return l;
}

STriangle Triangle::localToScreen(Camera &c)
{
    return STriangle(c.localToScreen(v0), c.localToScreen(v1), c.localToScreen(v2));
}

V3 Triangle::normal()
{
    V3 a = V3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    V3 b = V3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
    V3 n = V3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    float l = pow(n.x * n.x + n.y * n.y + n.z * n.z, .5);
    n.x /= l;
    n.y /= l;
    n.z /= l;
    return n;
}
