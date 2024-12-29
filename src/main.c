/*
 * This example creates an SDL window and renderer, and then draws some
 * geometry (arbitrary polygons) to it every frame.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_opengl.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "render/render.h"
#include "render/window.h"

struct WindowContext win;

SDL_Texture *text = NULL;
static int texture_width = 0;
static int texture_height = 0;
bool clickingLMB = false;
GLuint vshader = 0;

GLuint fshader = 0;
GLuint pobject = 0;

GLuint jfashader = 0;
GLuint jfaobject = 0;

GLuint distshader = 0;
GLuint distobject = 0;

GLuint simpleshader = 0;
GLuint simpleobject = 0;

GLuint framebuffer;

struct ScreenGeometry geo;
Uint64 fps;
Uint64 fpsCounter;

int colors[][3] = {
    {0, 0, 0},
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
};
int colorIndex = 0;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    win = win_initSDLContext();

    vshader = ren_createShader("../src/shader/shader.vert", GL_VERTEX_SHADER);

    fshader = ren_createShader("../src/shader/smart_raymarch.frag", GL_FRAGMENT_SHADER);
    jfashader = ren_createShader("../src/shader/jfa.frag", GL_FRAGMENT_SHADER);
    distshader = ren_createShader("../src/shader/jfa_dist.frag", GL_FRAGMENT_SHADER);
    simpleshader = ren_createShader("../src/shader/simple.frag", GL_FRAGMENT_SHADER);

    pobject = ren_createProgram((GLuint[]){vshader, fshader});
    // set texture uniform positions
    glUseProgram(pobject);
    glUniform1i(glGetUniformLocation(pobject, "distanceTexture"), 0);
    glUniform1i(glGetUniformLocation(pobject, "ourTexture"),  1);
    glUniform1i(glGetUniformLocation(pobject, "lastTexture"),  2);

    jfaobject = ren_createProgram((GLuint[]){vshader, jfashader});
    distobject = ren_createProgram((GLuint[]){vshader, distshader});
    simpleobject = ren_createProgram((GLuint[]){vshader, simpleshader});

    geo = win_initScreenGeometry();
    
    glGenFramebuffers(1, &framebuffer);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }

    if (event->key.down) {
        if (event->key.key == SDLK_SPACE) {
            colorIndex++;
            if (colorIndex + 1 > sizeof(colors) / sizeof(colors[0])) {
                colorIndex = 0;
            }
            printf ("switched to color %d, %d, %d\n", colors[colorIndex][0], colors[colorIndex][1], colors[colorIndex][2]);
        }
    }

    // mouse
    if (event->button.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        clickingLMB = true;
    }else if (event->button.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        clickingLMB = false;
    }

    if (event->motion.type == SDL_EVENT_MOUSE_MOTION
        || event->button.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (clickingLMB) {
            int x_pos = event->motion.x;
            int y_pos = WINDOW_HEIGHT - event->motion.y;
            int radius = 5;
            for (int i = x_pos - radius; i < (x_pos + radius) - 1; i ++) {
                for (int j = y_pos - radius; j < (y_pos + radius) - 1; j ++) {
                    if (j < 1 || i < 1) {
                        break;
                    }
                    if (sqrt(pow(i - x_pos, 2) + pow(j - y_pos, 2)) < radius) {
                        SDL_WriteSurfacePixel(win.surface, i, j, colors[colorIndex][0], colors[colorIndex][1], colors[colorIndex][2], 255);
                    }
                }
            }
        }
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    int Mode = GL_RGBA; // win.surface->format == SDL_PIXELFORMAT_RGBA32
    
    glOrtho(0,win.surface->w,win.surface->h,0,-1,1); //Set the matrix
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);

    GLuint input_texture = ren_createTexture(); // create texture to read from (input)
    // // bind sdl surface to it
    glTexImage2D(GL_TEXTURE_2D, 0, Mode, win.surface->w, win.surface->h, 0, Mode, GL_UNSIGNED_BYTE, win.surface->pixels);

    GLuint output_texture = ren_createTexture();
    glBindTexture(GL_TEXTURE_2D, output_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, Mode, win.surface->w, win.surface->h, 0, Mode, GL_UNSIGNED_BYTE, NULL);

    // set render target
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // set the output buffer texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_texture, 0);


    const int NUM_PASSES = ceil(log2(fmax(WINDOW_WIDTH, WINDOW_HEIGHT)));

    glUseProgram(jfaobject); // outside loop
    // todo can be cached
    GLuint uOffset = glGetUniformLocation(jfaobject, "uOffset");

    glUniform2f(glGetUniformLocation(jfaobject, "oneOverSize"),
        (1.0 / (float)win.surface->w), (1.0 / (float)win.surface->h));

    GLuint isSeed = glGetUniformLocation(jfaobject, "isSeed");
    glUniform1f(isSeed, true);  

    GLuint input_dead_textures[NUM_PASSES - 2];

    // -2 looks great, no idea why
    for (int i = 0; i < NUM_PASSES - 2; i ++) {
        // assign sampler texture
        glBindTexture(GL_TEXTURE_2D, input_texture);

        GLfloat lightPos = pow(2, NUM_PASSES - i - 1);
        glUniform1f(uOffset, lightPos);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        input_dead_textures[i] = input_texture;
        input_texture = output_texture; // TODO THIS IS LEAKY, use array

        if (i == 0) {
            glUniform1f(isSeed, false);
        }
    }

    //distance renderpass
    glUseProgram(distobject);  
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // last renderpass (gi)

    GLuint original_input_texture = ren_createTexture(); // create texture to read from (input)
    // bind sdl surface to it
    glTexImage2D(GL_TEXTURE_2D, 0, Mode, win.surface->w, win.surface->h, 0, Mode, GL_UNSIGNED_BYTE, win.surface->pixels);

    GLuint gi_output_texture = ren_createTexture(); 

    glBindTexture(GL_TEXTURE_2D, gi_output_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, Mode, win.surface->w, win.surface->h, 0, Mode, GL_UNSIGNED_BYTE, NULL);

    // set render target
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // set the output buffer texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gi_output_texture, 0);

    // Activate texture unit 1 and bind the second texture
    glActiveTexture(GL_TEXTURE2); // last tex
    glBindTexture(GL_TEXTURE_2D, original_input_texture);

    // Activate texture unit 1 and bind the second texture
    glActiveTexture(GL_TEXTURE1); // current tex
    glBindTexture(GL_TEXTURE_2D, original_input_texture);

    // Activate texture unit 0 and bind the first texture
    glActiveTexture(GL_TEXTURE0); // dist
    glBindTexture(GL_TEXTURE_2D, output_texture);



    GLuint gi_dead_textures[200];
    GLuint gi_output_dead_textures[200];
    gi_output_dead_textures[0] = gi_output_texture;
    gi_dead_textures[0] = original_input_texture;

    // Use the shader program
    glUseProgram(pobject);

    const int BASE_RAY_COUNT = 32;

    GLuint uRayCount = glGetUniformLocation(pobject, "rayCount");
    GLuint uResolution = glGetUniformLocation(pobject, "resolution");
    glUniform1i(glGetUniformLocation(pobject, "baseRayCount"), BASE_RAY_COUNT);
    glUniform2f(uResolution, WINDOW_WIDTH, WINDOW_HEIGHT);  

    for (int i = 2; i >= 1; i--) {
        original_input_texture = ren_createTexture(); // create texture to read from (input)
        // bind sdl surface to it
        glTexImage2D(GL_TEXTURE_2D, 0, Mode, win.surface->w, win.surface->h, 0, Mode, GL_UNSIGNED_BYTE, win.surface->pixels);


        glUniform1i(uRayCount, (int)pow(BASE_RAY_COUNT, i));

        GLuint gi_new_output_texture = ren_createTexture(); 

        glBindTexture(GL_TEXTURE_2D, gi_new_output_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, Mode, win.surface->w, win.surface->h, 0, Mode, GL_UNSIGNED_BYTE, NULL);

        // set render target
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        // set the output buffer texture

        gi_output_dead_textures[i] = gi_new_output_texture;

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, original_input_texture); // last pass
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gi_dead_textures[0]); // original
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, output_texture); // disr

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        gi_dead_textures[i] = original_input_texture;
        original_input_texture = gi_output_texture; // TODO THIS IS LEAKY, use array
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind screen, and render out final
    glUseProgram(simpleobject);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gi_output_texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    SDL_GL_SwapWindow(win.window);
    glDeleteFramebuffers(1, &framebuffer);


    // gi_output_texture implicitly freed via original_input_texture
    glDeleteTextures(4, (GLuint[]){input_texture, original_input_texture, output_texture});

    glDeleteTextures(NUM_PASSES - 2, gi_dead_textures);
    glDeleteTextures(NUM_PASSES - 2, gi_output_dead_textures);
    glDeleteTextures(NUM_PASSES - 2, input_dead_textures);

    SDL_Delay(15);

    fpsCounter++;
    if (SDL_GetTicks() - fps > 1000) {
        printf("%ld fps\n", fpsCounter);
        fps = SDL_GetTicks();
        fpsCounter = 0;
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_DestroyTexture(win.texture);
    SDL_DestroySurface(win.surface);

    // Once finished with OpenGL functions, the SDL_GLContext can be destroyed.
    SDL_GL_DestroyContext(win.context);

    glDeleteVertexArrays(1, &geo.VAO);
    glDeleteBuffers(1, &geo.VBO);
    glDeleteBuffers(1, &geo.EBO);

    /* SDL will clean up the window/renderer for us. */
}
