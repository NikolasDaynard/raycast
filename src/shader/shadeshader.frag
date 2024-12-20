#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

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
    const int rayCount = 8;
    const int maxSteps = 640;
    const float PI = 3.14159265;
    const float TAU = 2.0 * PI;
    float oneOverRayCount = 1.0 / float(rayCount);
    float tauOverRayCount = TAU * oneOverRayCount;

    // Distinct random value for every pixel
    float noise = rand(TexCoord);

    vec4 radiance = vec4(0.0);

    for(int i = 0; i < rayCount; i++) {
        float angle = tauOverRayCount * (float(i) + noise);
        vec2 rayDirectionUv = vec2(cos(angle), -sin(angle)) / (640.0); // 100.0 is size TODO

        // Our current position, plus one step.
        vec2 sampleUv = TexCoord + rayDirectionUv;

        for (int step = 0; step < maxSteps; step++) {
            if (outOfBounds(sampleUv)) break;
            vec4 sampleLight = texture(ourTexture, sampleUv);
            if (sampleLight.a > 0.1) {
                radiance += sampleLight;
                break;
            }

            sampleUv += rayDirectionUv;
        }
    }
    return radiance * oneOverRayCount;
}


void main()
{
    FragColor = vec4(texture(ourTexture, TexCoord).rgb * TexCoord.x, 1.0);
    // FragColor = vec4(TexCoord.x, TexCoord.y, 0.0, 0.1);
    // gl_FragColor = vec4(raymarch().rgb, 1.0);
    // gl_FragColor = vec4(1.0, 0.5, 1.0, 1.0);
}