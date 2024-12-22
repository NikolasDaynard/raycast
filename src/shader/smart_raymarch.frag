#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D distanceTexture;
uniform sampler2D ourTexture; // input
uniform sampler2D lastTexture;
uniform int rayCount;
uniform int resolution;

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
    const int maxSteps = 40;
    const float PI = 3.14159265;
    const float TAU = 2.0 * PI;
    float oneOverRayCount = 1.0 / float(rayCount);
    float tauOverRayCount = TAU * oneOverRayCount;

    vec2 coord = uv * resolution;

    bool isLastLayer = rayCount == baseRayCount;
    vec2 effectiveUv = isLastLayer ? uv : floor(coord / 2.0) * 2.0 / resolution;

    // Distinct random value for every pixel
    float noise = 1.0;

    vec4 radiance = vec4(0.0);

    for(int i = 0; i < rayCount; i++) {
        float angle = tauOverRayCount * (float(i) + noise);
        vec2 rayDirectionUv = vec2(cos(angle), -sin(angle)); // 100.0 is size TODO

        // Our current position, plus one step.
        vec2 sampleUv = TexCoord;

        for (int step = 1; step < maxSteps; step++) {
            // How far away is the nearest object?
            float dist = texture(distanceTexture, sampleUv).r;

            // Go the direction we're traveling (with noise)
            sampleUv += rayDirectionUv * dist;

            if (outOfBounds(sampleUv)) break;

            // We hit something! (EPS = small number, like 0.001)
            if (dist < 0.001) {
                // Collect the radiance
                radiance += texture(ourTexture, sampleUv);
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

// #version 330 core

// uniform vec2 resolution = vec2(640.0, 480.0);
// uniform sampler2D ourTexture; // original
// uniform sampler2D distanceTexture;
// uniform sampler2D lastTexture; // lastpass
// uniform bool enableSun;
// uniform int rayCount;
// uniform int baseRayCount = 16;
// uniform float sunAngle;
// uniform float intervalPartial;

// in vec3 ourColor;
// in vec2 TexCoord;
// out vec4 FragColor;

// const float PI = 3.14159265;
// const float TAU = 2.0 * PI;
// const float srgb = 2.2;

// const vec3 skyColor = vec3(0.02, 0.08, 0.2);
// const vec3 sunColor = vec3(0.95, 0.95, 0.9);
// const int maxSteps = 32;
// const float EPS = 0.001;

// vec3 sunAndSky(float rayAngle) {
//   // Get the sun / ray relative angle
//   float angleToSun = mod(rayAngle - sunAngle, TAU);

//   // Sun falloff based on the angle
//   float sunIntensity = smoothstep(1.0, 0.0, angleToSun);

//   // And that's our sky radiance
//   return sunColor * sunIntensity + skyColor;
// }

// bool outOfBounds(vec2 uv) {
//   return uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;
// }

// void main() {
//   vec2 uv = TexCoord;

//   vec2 cascadeExtent = resolution;
//   vec2 coord = floor(TexCoord * cascadeExtent);

//   vec4 radiance = vec4(0.0);

//   float oneOverRayCount = 1.0 / float(rayCount);
//   float angleStepSize = TAU * oneOverRayCount;

//   float intervalStart = rayCount == baseRayCount ? 0.0 : intervalPartial;
//   float intervalEnd = rayCount == baseRayCount ? intervalPartial : sqrt(2.0);

//   vec2 effectiveUv = rayCount == baseRayCount ? TexCoord : (floor(coord / 2.0) * 2.0) / resolution;

//   vec2 scale = min(resolution.x, resolution.y) / resolution;
//   vec2 oneOverSize = 1.0 / resolution;
//   float minStepSize = min(oneOverSize.x, oneOverSize.y) * 0.5;

//   // Shoot rays in "rayCount" directions, equally spaced.
//   for (int i = 0; i < rayCount; i++) {
//       float index = float(i);
//       float angleStep = (index + 0.5);
//       float angle = angleStepSize * angleStep;
//       vec2 rayDirection = vec2(cos(angle), -sin(angle));

//       vec2 sampleUv = effectiveUv + rayDirection * intervalStart * scale;
//       vec4 radDelta = vec4(0.0);

//       float traveled = intervalStart;

//       // We tested uv already (we know we aren't an object), so skip step 0.
//       for (int step = 1; step < maxSteps; step++) {
//           // How far away is the nearest object?
//           float dist = texture(distanceTexture, sampleUv).r;

//           // Go the direction we're traveling
//           sampleUv += rayDirection * dist * scale;

//           if (outOfBounds(sampleUv)) break;

//           if (dist < minStepSize) {
//               vec4 colorSample = texture(ourTexture, sampleUv);
//               radDelta += vec4(pow(colorSample.rgb, vec3(srgb)), 1.0);
//               break;
//           }

//           traveled += dist;
//           if (traveled >= intervalEnd) break;
//       }

//       // Only merge on non-opaque areas
//       if (rayCount == baseRayCount && radDelta.a == 0.0) {
//         vec4 upperSample = texture(lastTexture, uv);

//         radDelta += vec4(pow(upperSample.rgb, vec3(srgb)), upperSample.a);

//         // If we didn't find an object, add some sky + sun color
//         if (enableSun) {
//           radDelta += vec4(sunAndSky(angle), 1.0);
//         }
//       }

//       // Accumulate total radiance
//       radiance += radDelta;
//   }

//   vec3 final = (radiance.rgb * oneOverRayCount);

//   FragColor = vec4(pow(final, vec3(1.0 / srgb)), 1.0);
// }