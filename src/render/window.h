/**
 *  Defines some helpers for windows in sdl
 */ 

#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>
#include <stdio.h>

struct WindowContext {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Surface *surface; // stores the radience state
    SDL_GLContext context;
};

/**
 * @return A new WindowContext
 */
struct WindowContext win_initSDLContext();

struct ScreenGeometry {
    unsigned int VBO, VAO, EBO;
};

/**
 * @return A new WindowContext
 */
struct ScreenGeometry win_initScreenGeometry();
