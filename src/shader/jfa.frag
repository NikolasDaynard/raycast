#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

uniform vec2 oneOverSize;
uniform float uOffset;
uniform bool isSeed;

void main() {

    if (isSeed) {
        gl_FragColor = vec4(TexCoord * texture(ourTexture, TexCoord).a, 0.0, 1.0);
    } else {
        vec4 nearestSeed = vec4(-2.0);
        float nearestDist = 999999.9;
        
        for (float y = -1.0; y <= 1.0; y += 1.0) {
        for (float x = -1.0; x <= 1.0; x += 1.0) {
            vec2 sampleUV = TexCoord + vec2(x, y) * uOffset * oneOverSize;
            
            // Check if the sample is within bounds
            if (sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0) { continue; }
            
            vec4 sampleValue = texture(ourTexture, sampleUV);
            vec2 sampleSeed = sampleValue.xy;
            
                if (sampleSeed.x != 0.0 || sampleSeed.y != 0.0) {
                    vec2 diff = sampleSeed - TexCoord;
                    float dist = dot(diff, diff);
                    if (dist < nearestDist) {
                        nearestDist = dist;
                        nearestSeed = sampleValue;
                    }
                }
            }
        }

        gl_FragColor = nearestSeed;
    }
}