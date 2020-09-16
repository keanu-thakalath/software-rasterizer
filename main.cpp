#include "Math.h"
#include "3DG.h"
#include "RenderEngine.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <algorithm>
#include <iostream>
#include <string>

const int SCREEN_X = 1600;
const int SCREEN_Y = 900;
const int FOV = 90;
const int SENSITIVITY = 50;
const float MOVEMENT_SPEED = .07;

int main(int argc, char* argv[])
{
    RenderEngine engine = RenderEngine();
    // render a .obj file
    std::string filename;
    if (argc < 2)
    {
        printf("\"filename\" parameter is missing, using \"objects/monkey\"\n");
        filename = "objects/monkey";
    }
    else
    {
        filename = argv[1];
    }

    if (!engine.init("3D", SCREEN_X, SCREEN_Y, FOV))
    {
        printf("SDL could not initialize!\n");
        return 1;
    }

    // load triangles from .obj file
    Object obj = Object(filename);
    if (!obj.loadObj())
    {
        printf("Error: Could not load file \"%s\"\n", filename.c_str());
        return 1;
    }

    // hide mouse
    engine.hideCursor();

    bool quit = false;
    bool pressedELastFrame = false;
    bool lockMouse = true;
    SDL_Event e;

    int mouseX, mouseY;

    while (!quit)
    {
        bool mouseMoved = false;

        // get events
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_MOUSEMOTION)
            {
                SDL_GetMouseState(&mouseX, &mouseY);
                mouseMoved = true;
            }
        }

        // get key presses
        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
        bool pressingRight = currentKeyStates[SDL_SCANCODE_RIGHT] || currentKeyStates[SDL_SCANCODE_D];
        bool pressingLeft = currentKeyStates[SDL_SCANCODE_LEFT] || currentKeyStates[SDL_SCANCODE_A];
        bool pressingUp = currentKeyStates[SDL_SCANCODE_UP] || currentKeyStates[SDL_SCANCODE_W];
        bool pressingDown = currentKeyStates[SDL_SCANCODE_DOWN] || currentKeyStates[SDL_SCANCODE_S];
        bool pressingSpace = currentKeyStates[SDL_SCANCODE_SPACE];
        bool pressingShift = currentKeyStates[SDL_SCANCODE_LSHIFT];
        bool pressingR = currentKeyStates[SDL_SCANCODE_R];
        bool pressingE = currentKeyStates[SDL_SCANCODE_E];
        bool pressingEscape = currentKeyStates[SDL_SCANCODE_ESCAPE];

        // handle movement
        float dx = 0;
        float dy = 0;
        float dz = 0;

        // adjust the movement speed to account for fps
        float ms = engine.mspf * MOVEMENT_SPEED / 16.66f;
        if (pressingRight)
        { dx += ms; }
        if (pressingLeft)
        { dx -= ms; }
        if (pressingUp)
        { dz += ms; }
        if (pressingDown)
        { dz -= ms;}
        if (pressingSpace)
        { dy += ms; }
        if (pressingShift)
        { dy -= ms; }
        if (pressingEscape)
        { quit = true; }

        // lock or unlock the mouse
        if (pressingE && !pressedELastFrame)
        {
            lockMouse = !lockMouse;
            if (lockMouse)
            {
                engine.hideCursor();
            }
            else
            {
                engine.showCursor();
            }
        }
        pressedELastFrame = pressingE;

        // move the camera with game mechanics - dx does not follow the x vector, same with dy and dz
        engine.c.moveGM(dx, dy, dz);

        // if the user presses e they will free their mouse
        if (lockMouse)
        { engine.resetMouse(); }

        // handle rotation from mouse movement
        if (mouseMoved && lockMouse)
        {
            // rotate the camera with game mechanics - instead of rotating x and z around y, rotate x y and z around <0, 1, 0>
            engine.c.rotateXGM(SENSITIVITY * (2.0f * (mouseX - SCREEN_X) / SCREEN_X + 1));
            engine.c.rotateY(SENSITIVITY * (2.0f * (mouseY - SCREEN_Y) / SCREEN_Y + 1));
        }

        // R key sets position and direction to <0, 0, 0>, +Z
        if (pressingR)
        { engine.c.resetPosition(); }

        // if the camera position changed invert the matrix
        if (dx != 0 || dy != 0 || dz != 0 || mouseMoved)
        { engine.c.invert(); }

        engine.newFrame();
        engine.renderObjSmooth(obj);
        engine.endFrame();
        engine.printMSPF();
    }
}
