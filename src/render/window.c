#include "window.h"

struct WindowContext win_initSDLContext() {
    struct WindowContext new_win;

    new_win.surface = SDL_CreateSurface(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_PIXELFORMAT_RGBA32);
    
    if (new_win.surface == NULL) {
        printf("%s", SDL_GetError());
    }

    for (int i = 0; i < WINDOW_WIDTH - 1; i ++) {
        for (int j = 0; j < WINDOW_HEIGHT - 1; j ++) {
            SDL_WriteSurfacePixel(new_win.surface, i, j, 0, 0, 0, 0);
        }
    }

    SDL_SetAppMetadata("Simple Radience Cascade Renderer", "0.0.1", "com.example.renderer-geometry");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return new_win;
    }
    
    //SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE
    if (!SDL_CreateWindowAndRenderer("examples/renderer/geometry", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL, &new_win.window, &new_win.renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return new_win;
    }

    new_win.context = SDL_GL_CreateContext(new_win.window);

    return new_win;
}


struct ScreenGeometry win_initScreenGeometry() {

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

    struct ScreenGeometry new_geo;
    glGenVertexArrays(1, &new_geo.VAO);
    glGenBuffers(1, &new_geo.VBO);
    glGenBuffers(1, &new_geo.EBO);

    glBindVertexArray(new_geo.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, new_geo.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_geo.EBO);
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

    return new_geo;
}