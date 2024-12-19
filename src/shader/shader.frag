#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord * 200.0);
    // FragColor = vec4(0.2, 0.2, 0.2, 1.0);
}