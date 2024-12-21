#include <malloc.h>
#include <stdio.h>
#include "render.h"

GLuint ren_createTexture() {
    GLuint new_texture; // create init texture
    glGenTextures(1, &new_texture);
    glBindTexture(GL_TEXTURE_2D, new_texture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // these performance bad
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return new_texture;
}

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
        printf("ERROR Cannot open shader file!\n");
  		  return 0;
    }
    int bytesinfile = filelength(file);
    unsigned char *buffer = (unsigned char*)malloc(bytesinfile+1);
    int bytesread = fread( buffer, 1, bytesinfile, file);
    buffer[bytesread] = 0; // Terminate the string with 0
    fclose(file);
    
    return buffer;
}

GLuint ren_createShader(const char *filename, GLenum type) {
    GLuint new_shader = glCreateShader(type);

    const char *shaderStrings[1];
    unsigned char *shaderAssembly = readShaderFile(filename);
    shaderStrings[0] = (char*)shaderAssembly;
    glShaderSource(new_shader, 1, shaderStrings, NULL);
    glCompileShader(new_shader);
    free((void *)shaderAssembly);

    GLint success = 0;
    glGetShaderiv(new_shader, GL_COMPILE_STATUS, &success);

    if(success == GL_FALSE)
  	{
        char str[4096];
        glGetShaderInfoLog(new_shader, sizeof(str), NULL, str);

        switch (type) {
            case GL_VERTEX_SHADER:
                printf("Vertex s");
                break;
            case GL_FRAGMENT_SHADER:
                printf("Fragment s");
                break;
            case GL_COMPUTE_SHADER:
                printf("Compute s");
                break;
            default:
                printf("S");
                break;
        }
 
        printf("hader compile error %sin file %s\n", str, filename);
  	}

    return new_shader;
}

GLuint ren_createProgram(GLuint *shaders) {
    GLuint new_program = glCreateProgram();

    for (int i = 0; i < sizeof(shaders) / sizeof(shaders[0]); i++) {
        glAttachShader(new_program, shaders[i]);
    }

    glLinkProgram(new_program);

    GLint shadersLinked;
    glGetProgramiv( new_program, GL_LINK_STATUS, &shadersLinked );

    if( shadersLinked == GL_FALSE )
	{
        char str[4096];
		glGetProgramInfoLog( new_program, sizeof(str), NULL, str );
		printf("Program object linking error %s\n", str);
	}

    return new_program;
}