/*
 * This example creates an SDL window and renderer, and then draws some
 * geometry (arbitrary polygons) to it every frame.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#define GL_GLEXT_PROTOTYPES 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_opengl.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "render/render.h"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Surface *surface = NULL; // stores the radience state
static SDL_GLContext context = NULL;
SDL_Texture *text = NULL;
static int texture_width = 0;
static int texture_height = 0;
bool clickingLMB = false;
GLuint vshader = 0;
GLuint fshader = 0;
GLuint shadeshader = 0;
GLuint pobject = 0;
GLuint shadeobject = 0;
GLuint framebuffer;
float vertices[] = {
    // positions          // colors           // texture coords
    1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
    1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
};
unsigned int indices[] = {  
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};
unsigned int VBO, VAO, EBO;
Uint64 fps;
Uint64 fpsCounter;

int colors[][3] = {
    {0, 0, 0},
    {128, 0, 0},
    {255, 0, 255},
    {255, 255, 255},
    {255, 128, 128},
};
int colorIndex = 0;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    surface = SDL_CreateSurface(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_PIXELFORMAT_RGBA32);
    
    if (surface == NULL) {
        printf("%s", SDL_GetError());
    }

    for (int i = 0; i < WINDOW_WIDTH - 1; i ++) {
        for (int j = 0; j < WINDOW_HEIGHT - 1; j ++) {
            SDL_WriteSurfacePixel(surface, i, j, 0, 0, 0, 0);
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

    vshader = ren_createShader("../src/shader/shader.vert", GL_VERTEX_SHADER);

    fshader = ren_createShader("../src/shader/shader.frag", GL_FRAGMENT_SHADER);

    // Create a program object and attach the two compiled shaders.
    pobject = glCreateProgram();
    glAttachShader( pobject, vshader );
    glAttachShader( pobject, fshader );

    // Link the program object and print out the info log.
    glLinkProgram( pobject );

    GLint shadersLinked;
    glGetProgramiv( pobject, GL_LINK_STATUS, &shadersLinked );

    if( shadersLinked == GL_FALSE )
	{
        char str[4096];
		glGetProgramInfoLog( pobject, sizeof(str), NULL, str );
		printf("Program object linking error %s\n", str);
	}

    shadeshader = ren_createShader("../src/shader/shadeshader.frag", GL_FRAGMENT_SHADER);

    shadeobject = glCreateProgram();

    glAttachShader( shadeobject, vshader );
    glAttachShader( shadeobject, shadeshader );

    glLinkProgram( shadeobject );

    glGetProgramiv( shadeobject, GL_LINK_STATUS, &shadersLinked );

    if( shadersLinked == GL_FALSE )
	{
        char str[4096];
		glGetProgramInfoLog( shadeobject, sizeof(str), NULL, str );
		printf("Program object linking error %s\n", str);
	}

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glGenFramebuffers(1, &framebuffer);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }

    if (event->key.down) {
        // if (event->key.key == SDLK_LEFT) {
        //     TEST_X += 1;
        // }
        // if (event->key.key == SDLK_RIGHT) {
        //     TEST_X -= 1;
        // }
        if (event->key.key == SDLK_SPACE) {
            colorIndex++;
            if (colorIndex > sizeof(colors) / sizeof(colors[0])) {
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
            int radius = 15;
            for (int i = x_pos - radius; i < (x_pos + radius) - 1; i ++) {
                for (int j = y_pos - radius; j < (y_pos + radius) - 1; j ++) {
                    if (j < 1 || i < 1) {
                        break;
                    }
                    if (sqrt(pow(i - x_pos, 2) + pow(j - y_pos, 2)) < radius) {
                        SDL_WriteSurfacePixel(surface, i, j, colors[colorIndex][0], colors[colorIndex][1], colors[colorIndex][2], 255);
                    }
                }
            }

            const int passes = ceil(log2(fmax(WINDOW_WIDTH, WINDOW_HEIGHT)));
            

            // renderA 
            // renderB
            // let currentOutput = renderA;

            // for (let i = 0; i < passes; i++) {
            // plane.material.uniforms.inputTexture.value = currentInput;
            // plane.material.uniforms.uOffset.value = Math.pow(2, passes - i - 1);

            // renderer.setRenderTarget(currentOutput);
            // render();

            // currentInput = currentOutput.texture;
            // currentOutput = (currentOutput === renderA) ? renderB : renderA;
            // }


        }
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
   
    int Mode = GL_RGB;
    
    if(surface->format == SDL_PIXELFORMAT_RGBA32) {
        Mode = GL_RGBA;
    }
    

    // For Ortho mode, of course
    // glViewport(-WINDOW_WIDTH, -WINDOW_HEIGHT, WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2);
    glOrtho(0,surface->w,surface->h,0,-1,1); //Set the matrix
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);

    GLuint input_texture = ren_createTexture(); // create texture to read from (input)
    // bind sdl surface to it
    glTexImage2D(GL_TEXTURE_2D, 0, Mode, surface->w, surface->h, 0, Mode, GL_UNSIGNED_BYTE, surface->pixels);

    GLuint output_texture = ren_createTexture(); // create texture to write to (output)
    // bind new empty texture
    glTexImage2D(GL_TEXTURE_2D, 0, Mode, surface->w, surface->h, 0, Mode, GL_UNSIGNED_BYTE, NULL);

    // set render target
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // set the output buffer texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_texture, 0);

    // assign sampler texture
    glBindTexture(GL_TEXTURE_2D, input_texture);

    glUseProgram(pobject);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // second renderpass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, output_texture);

    glUseProgram(shadeobject);  

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    SDL_GL_SwapWindow(window);

    glDeleteFramebuffers(1, &framebuffer);  






    SDL_Delay(15);
    // printf("%d click \n", clickingLMB);
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
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);

    // Once finished with OpenGL functions, the SDL_GLContext can be destroyed.
    SDL_GL_DestroyContext(context);  

    /* SDL will clean up the window/renderer for us. */
}
