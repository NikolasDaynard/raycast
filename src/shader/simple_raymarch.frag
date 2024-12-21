#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D distanceTexture;
uniform sampler2D ourTexture; // original input

bool outOfBounds(vec2 uv) {
  return uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;
}

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

vec4 raymarch() {
    vec4 light = texture(ourTexture, TexCoord);

    if (light.a > 0.1) {
        return light;
    }
    const int rayCount = 32;
    const int maxSteps = 40;
    const float PI = 3.14159265;
    const float TAU = 2.0 * PI;
    float oneOverRayCount = 1.0 / float(rayCount);
    float tauOverRayCount = TAU * oneOverRayCount;

    // Distinct random value for every pixel
    float noise = rand(TexCoord);

    vec4 radiance = vec4(0.0);

    for(int i = 0; i < rayCount; i++) {
        float angle = tauOverRayCount * (float(i) + noise);
        vec2 rayDirectionUv = vec2(cos(angle), -sin(angle)); // 100.0 is size TODO

        // Our current position, plus one step.
        vec2 sampleUv = TexCoord + (rayDirectionUv / 320.0);// = TexCoord + rayDirectionUv;

        for (int step = 1; step < maxSteps; step++) {
            // How far away is the nearest object?
            float dist = texture(distanceTexture, sampleUv).r;

            // Go the direction we're traveling (with noise)
            sampleUv += rayDirectionUv * dist;

            if (outOfBounds(sampleUv)) break;

            // We hit something! (EPS = small number, like 0.001)
            if (dist < 0.001 || step == maxSteps - 2) {
                // Collect the radiance
                radiance += texture(ourTexture, sampleUv + rayDirectionUv / 320.0);
                break;
            }
        }
    }
    
    return radiance * oneOverRayCount;
}


void main()
{
    // FragColor = (texture(ourTexture, TexCoord) + texture(ourTexture, TexCoord + vec2(0.1, 0.1))) / 2.0;
    // FragColor = texture(ourTexture, TexCoord) + 0.1;
    // FragColor = texture(distanceTexture, TexCoord);
    // FragColor = vec4(TexCoord.x, TexCoord.y, 0.2, 1.0);
    FragColor = vec4(raymarch().rgb, 1.0);
}