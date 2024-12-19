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

#include "raycasting/raycast.c"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Surface *surface = NULL; // stores the radience state
static SDL_GLContext context = NULL;
static int texture_width = 0;
static int texture_height = 0;
bool clickingLMB = false;
GLuint vshader = 0;
GLuint fshader = 0;
GLuint pobject = 0;
GLuint quadList;

long filelength(FILE *file) {
    long numbytes;
    long savedpos = ftell(file);
    fseek(file, 0, SEEK_END);
    numbytes = ftell(file);
    fseek(file, savedpos, SEEK_SET);
    return numbytes;
}

unsigned char* readShaderFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if(file == NULL)
    {
        printf("ERROR Cannot open shader file!");
  		  return 0;
    }
    int bytesinfile = filelength(file);
    unsigned char *buffer = (unsigned char*)malloc(bytesinfile+1);
    int bytesread = fread( buffer, 1, bytesinfile, file);
    buffer[bytesread] = 0; // Terminate the string with 0
    fclose(file);
    
    return buffer;
}

void initQuadList(GLuint *listID)
{
  *listID = glGenLists(1);
  
  glNewList(*listID, GL_COMPILE);
    glColor3f(1.0f, 1.0f, 1.0f); // White base color
    glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(-1.0f, -1.0f, 0.0f);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(1.0f, -1.0f, 0.0f);
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(-1.0f, 1.0f, 0.0f);
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(1.0f, 1.0f, 0.0f);
    glEnd();
  glEndList();
}

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

    vshader = glCreateShader(GL_VERTEX_SHADER);

    const char *vertexShaderStrings[1];
    // glShaderSource(shader, GLsizei count, const GLchar **string, const GLint *length);
    unsigned char *vertexShaderAssembly = readShaderFile( "../src/shader/shader.vert" );
    vertexShaderStrings[0] = (char*)vertexShaderAssembly;
    glShaderSource(vshader, 1, vertexShaderStrings, NULL);
    glCompileShader(vshader);
    free((void *)vertexShaderAssembly);

    GLint success = 0;
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);

    if(success  == GL_FALSE)
  	{
        char str[4096];
        glGetShaderInfoLog(vshader, sizeof(str), NULL, str);
        printf("Vert shader compile error %s\n", str);
  	}

    fshader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *fragShaderStrings[1];
    // glShaderSource(shader, GLsizei count, const GLchar **string, const GLint *length);
    unsigned char *fragShaderAssembly = readShaderFile( "../src/shader/shader.frag" );
    fragShaderStrings[0] = (char*)fragShaderAssembly;
    glShaderSource(fshader, 1, fragShaderStrings, NULL);
    glCompileShader(fshader);
    free((void *)fragShaderAssembly);

    success = 0;
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);

    if(success  == GL_FALSE)
  	{
        char str[4096];
        glGetShaderInfoLog(fshader, sizeof(str), NULL, str);
        printf("Frag shader compile error %s\n", str);
  	}

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

    initQuadList(&quadList);

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
                        SDL_WriteSurfacePixel(surface, i, j, 128, 0, 0, 255);
                    }
                }
            }
        }
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

void setupCamera() {

    int width = WINDOW_WIDTH;
    int height = WINDOW_HEIGHT;
    
    // Get window size. It may start out different from the requested
    // size, and will change if the user resizes the window.

    // Set viewport. This is the pixel rectangle we want to draw into.
    glViewport( 0, 0, width, height ); // The entire window

    // Select and setup the projection matrix.
    glMatrixMode(GL_PROJECTION); // "We want to edit the projection matrix"
    // Select and setup the projection matrix.
    glMatrixMode(GL_PROJECTION); // "We want to edit the projection matrix"
    glLoadIdentity(); // Reset the matrix to identity
    // 20 degrees FOV, same aspect ratio as viewport, depth range 1 to 100
    // gluPerspective( 15.0f, (GLfloat)width/(GLfloat)height, 1.0f, 100.0f );
    // identi
    

    // Select and setup the modelview matrix.
    glMatrixMode( GL_MODELVIEW ); // "We want to edit the modelview matrix"
    glLoadIdentity(); // Reset the matrix to identity
    // Look from 0,-4,0 towards 0,0,0 with Z as "up" in the image
    // gluLookAt( 0.0f, -8.0f, 1.0f,  // Eye position
    //            0.0f, 0.0f, -0.5f,   // View point
    //            0.0f, 0.0f, 1.0f ); // Up vector
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
        // glViewport(-WINDOW_WIDTH, -WINDOW_HEIGHT, WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2);
        glOrtho(0,surface->w,surface->h,0,-1,1); //Set the matrix
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram( pobject );

          glPushMatrix();
            // glTranslatef(0.0f, 3.0f, -1.5f);
            // glScalef(1.0f, 4.0f, 1.0f);
            glCallList( quadList );
        glPopMatrix();

        SDL_GL_SwapWindow(window);


        SDL_Delay(15);
        // printf("%d click \n", clickingLMB);

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
