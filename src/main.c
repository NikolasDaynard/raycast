/*
 * This example creates an SDL window and renderer, and then draws some
 * geometry (arbitrary polygons) to it every frame.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <math.h>
#include "raycasting/raycast.c"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static int texture_width = 0;
static int texture_height = 0;

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_Surface *surface = NULL;

    SDL_SetAppMetadata("Simple Radience Cascade Renderer", "0.0.1", "com.example.renderer-geometry");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/renderer/geometry", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    const Uint64 now = SDL_GetTicks();

    /* we'll have the triangle grow and shrink over a few seconds. */
    const float direction = ((now % 2000) >= 1000) ? 1.0f : -1.0f;
    const float scale = ((float) (((int) (now % 1000)) - 500) / 500.0f) * direction;
    const float size = 200.0f + (200.0f * scale);

    SDL_Vertex vertices[4];

    /* as you can see from this, rendering draws over whatever was drawn before it. */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);  /* black, full alpha */
    SDL_RenderClear(renderer);  /* start with a blank canvas. */

    // SDL_zeroa(vertices);
    // vertices[0].position.x = ((float) WINDOW_WIDTH) / 2.0f;
    // vertices[0].position.y = (((float) WINDOW_HEIGHT) - size) / 2.0f;
    // vertices[0].color.r = 1.0f;
    // vertices[0].color.a = 1.0f;
    // vertices[1].position.x = (((float) WINDOW_WIDTH) + size) / 2.0f;
    // vertices[1].position.y = (((float) WINDOW_HEIGHT) + size) / 2.0f;
    // vertices[1].color.g = 1.0f;
    // vertices[1].color.a = 1.0f;
    // vertices[2].position.x = (((float) WINDOW_WIDTH) - size) / 2.0f;
    // vertices[2].position.y = (((float) WINDOW_HEIGHT) + size) / 2.0f;
    // vertices[2].color.b = 1.0f;
    // vertices[2].color.a = 1.0f;

    // SDL_RenderGeometry(renderer, NULL, vertices, 3, NULL, 0);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_FPoint startingPoint; // light
    startingPoint.x = .5;
    startingPoint.y = .5;
    // startingPoint.y = ((float)y) / WINDOW_HEIGHT;
    // for (int x = 0; x < WINDOW_WIDTH; x++) {
    //     for (int y = 0; y < WINDOW_HEIGHT; y++) {
    for (int i = 0; i < 300; i++) {
        SDL_FPoint direction = {sin(((float)i) / WINDOW_WIDTH), 
            sin(((float)-i) / WINDOW_WIDTH)};
        SDL_FPoint point = raycastDir(startingPoint, direction);
        SDL_RenderPoint(renderer, point.x * WINDOW_WIDTH, point.y * WINDOW_HEIGHT);
    }
    //     }
    // }

    SDL_RenderPresent(renderer);  /* put it all on the screen! */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_DestroyTexture(texture);
    /* SDL will clean up the window/renderer for us. */
}
