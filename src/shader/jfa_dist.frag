#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture; // jfa output


void main() {
    vec2 nearestSeed = texture(ourTexture, TexCoord).xy;
    // Clamp by the size of our texture (1.0 in uv space).
    float distance = clamp(distance(TexCoord, nearestSeed), 0.0, 1.0);

    // Normalize and visualize the distance
    gl_FragColor = vec4(vec3(distance), 1.0);

}