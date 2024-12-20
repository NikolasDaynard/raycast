#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

const float RADIUS = 0.5;

const float SOFTNESS = 1.0;

const float blurSize = 1.0/100.0;
const vec2 screenSize = vec2(640.0, 480.0); 

void main() {

    vec4 texColor = vec4(0.0); // texture2D(ourTexture, TexCoord)
    texColor += texture2D(ourTexture, TexCoord - 4.0*blurSize) * 0.05;
    texColor += texture2D(ourTexture, TexCoord - 3.0*blurSize) * 0.09;
    texColor += texture2D(ourTexture, TexCoord - 2.0*blurSize) * 0.12;
    texColor += texture2D(ourTexture, TexCoord - blurSize) * 0.15;
    texColor += texture2D(ourTexture, TexCoord) * 0.16;
    texColor += texture2D(ourTexture, TexCoord + blurSize) * 0.15;
    texColor += texture2D(ourTexture, TexCoord + 2.0*blurSize) * 0.12;
    texColor += texture2D(ourTexture, TexCoord + 3.0*blurSize) * 0.09;
    texColor += texture2D(ourTexture, TexCoord + 4.0*blurSize) * 0.05;

    vec2 position = (gl_FragCoord.xy / screenSize.xy) - vec2(0.5);
    float len = length(position);

    float vignette = smoothstep(RADIUS, RADIUS-SOFTNESS, len);

    texColor.rgb = mix(texColor.rgb, texColor.rgb * vignette, 0.5);

    gl_FragColor = vec4(texColor.rgb, texColor.a);
}