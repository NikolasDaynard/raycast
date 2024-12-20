/**
 *  Defines some useful helpers
 */ 

#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>

/**
 * @return A new OpenGL texture
 * @note Creates a new opengl texture with *opinionated* params
 */
GLuint ren_createTexture();

/**
 * @param filename: The filename to link, TODO: must be escaped with ../src/shader/
 * @param type: The shader type (GL_VERTEX_SHADER|GL_FRAGMENT_SHADER|GL_COMPUTE_SHADER etc.)
 * @return A new OpenGL shader with specified type
 */
GLuint ren_createShader(const char *filename, GLenum type);

/**
 * @param shaders: The shaders to link
 * @return A new linked pipeline
 * 
 * @note A program is the opengl equivelent of a pipeline
 * @note Generally will take a vertex and fragment shader
 */
GLuint ren_createProgram(GLuint *shaders);