#include "render.h"
#include <stdio.h>

GLuint ren_createTexture() {
    GLuint new_texture; // create init texture
    glGenTextures(1, &new_texture);
    glBindTexture(GL_TEXTURE_2D, new_texture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // these performance bad
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

GLuint ren_createShader(const char *filename) {
    GLuint new_shader = glCreateShader(GL_VERTEX_SHADER);

    const char *vertexShaderStrings[1];
    unsigned char *vertexShaderAssembly = readShaderFile(filename);
    vertexShaderStrings[0] = (char*)vertexShaderAssembly;
    glShaderSource(new_shader, 1, vertexShaderStrings, NULL);
    glCompileShader(new_shader);
    free((void *)vertexShaderAssembly);

    GLint success = 0;
    glGetShaderiv(new_shader, GL_COMPILE_STATUS, &success);

    if(success  == GL_FALSE)
  	{
        char str[4096];
        glGetShaderInfoLog(new_shader, sizeof(str), NULL, str);
        printf("Shader compile error %s\n in file %s\n", str, filename);
  	}
}