/**
 *  Defines some useful helpers
 */ 

#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>
#include "render.c"


/**
 * Creates a new opengl texture
 * Returns the new texture id
 */
GLuint ren_createTexture();

/**
 * Creates a new opengl texture
 * Returns the new texture id
 */
GLuint ren_createShader();