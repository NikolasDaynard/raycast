/*
 * This example creates an SDL window and renderer, and then draws some
 * geometry (arbitrary polygons) to it every frame.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_opengl.h>
#include <stdio.h>
#include <math.h>
#include "raycasting/raycast.c"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Surface *surface = NULL; // stores the radience state
static SDL_GLContext context = NULL;
static int texture_width = 0;
static int texture_height = 0;
int TEST_X = 0;


/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    surface = SDL_CreateSurface(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_PIXELFORMAT_RGBA32);
    
    if (surface == NULL) {
        printf("%s", SDL_GetError());
    }

    for (int i = 0; i < WINDOW_WIDTH - 1; i ++) {
        for (int j = 0; j < WINDOW_HEIGHT - 1; j ++) {
            SDL_WriteSurfacePixel(surface, i, j, 128, 33, 192, 255);
        }
    }
    printf("w%d\n", surface->w);
    printf("h%d\n", surface->h);

    SDL_SetAppMetadata("Simple Radience Cascade Renderer", "0.0.1", "com.example.renderer-geometry");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    //SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE
    if (!SDL_CreateWindowAndRenderer("examples/renderer/geometry", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    context = SDL_GL_CreateContext(window);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }

    if (event->key.down) {
        if (event->key.key == SDLK_LEFT) {
            TEST_X += 1;
        }
        if (event->key.key == SDLK_RIGHT) {
            TEST_X -= 1;
        }
    }

    // mouse
    if (event->button.down) {
        printf("x%f\n", event->button.x);
        printf("y%f\n", event->button.y);
        
        //
        // SDL_WriteSurfacePixel
        SDL_WriteSurfacePixel(surface, event->button.x, event->button.y, 128, 0, 0, 255);
        for (int i = event->button.x; i < (event->button.x + 30) - 1; i ++) {
            for (int j = event->button.y; j < (event->button.y + 30) - 1; j ++) {
                if (j < 1 || i < 1) {
                    break;
                }
                SDL_WriteSurfacePixel(surface, i, j, 128, 0, 0, 255);
            }
        }
        // event->button.x
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    // SDL_Texture *target_texture = SDL_CreateTextureFromSurface(renderer, surface);
        GLuint TextureID = 0;
        glGenTextures(1, &TextureID);
        
        int Mode = GL_RGB;
        
        if(surface->format == SDL_PIXELFORMAT_RGBA32) {
            Mode = GL_RGBA;
        }
        

        // For Ortho mode, of course
        int X = 0;
        int Y = 0;
        float w = 320.f;
        float h = 240.f;
        glViewport(0, 0, w, h);
        // glOrtho(0,surface->w,surface->h,0,-1,1); //Set the matrix
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, TextureID);


        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, Mode, surface->w, surface->h, 0, Mode, GL_UNSIGNED_BYTE, surface->pixels);
        
        int W = 1;
        int H = 1;

        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex3f(X, Y, 0);
            glTexCoord2f(1, 0); glVertex3f(X + W, Y, 0);
            glTexCoord2f(1, 1); glVertex3f(X + W, Y + H, 0);
            glTexCoord2f(0, 1); glVertex3f(X, Y + H, 0);
        glEnd();

        SDL_GL_SwapWindow(window);


        SDL_Delay(15);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);

    // Once finished with OpenGL functions, the SDL_GLContext can be destroyed.
    SDL_GL_DestroyContext(context);  

    /* SDL will clean up the window/renderer for us. */
}
