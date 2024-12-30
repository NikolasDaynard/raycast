#version 330 core
out vec4 FragColor;
  
precision highp float;
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D distanceTexture;
uniform sampler2D ourTexture; // input
uniform sampler2D lastTexture;
uniform int rayCount;
uniform vec2 resolution;
uniform int baseRayCount;

bool outOfBounds(vec2 uv) {
  return uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;
}

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

vec4 raymarch() {
    vec4 light = texture(ourTexture, TexCoord);

    if (light.a > 0.1) {
      // avoid on-line if not base case
      if (rayCount == baseRayCount) {
        return light;
      }
    }
    const int maxSteps = 40;
    const float PI = 3.14159265;
    const float TAU = 2.0 * PI;
    const float minStepSize = .001;
    const float srgb = 1.0;//.8;
    vec2 scale = min(resolution.x, resolution.y) / resolution;
    float oneOverRayCount = 1.0 / float(rayCount);
    float tauOverRayCount = TAU * oneOverRayCount;
    float angleStepSize = TAU * oneOverRayCount;

    vec2 coord = floor(TexCoord * resolution);

    bool isLastLayer = rayCount == baseRayCount;
    vec2 effectiveUv = isLastLayer ? TexCoord : floor(coord / 2.0) * 2.0 / resolution;

    float partial = 0.125; // when to split, lower = higher quality

    float intervalStart = rayCount == baseRayCount ? 0.0 : partial;
    float intervalEnd = rayCount == baseRayCount ? partial : sqrt(2.0);

    float sqrtBase = sqrt(float(baseRayCount));
    // If our `baseRayCount` is 16, this is 4 on the upper cascade or 1 on the lower.
    float spacing = rayCount == baseRayCount ? 1.0 : sqrtBase;

    vec2 size = floor(resolution / spacing);
    vec2 probeRelativePosition = mod(coord, size);
    vec2 rayPos = floor(coord / size);
    float baseIndex = float(baseRayCount) * (rayPos.x + (spacing * rayPos.y));
    vec2 probeCenter = (probeRelativePosition + 0.5) * spacing;
    vec2 normalizedProbeCenter = probeCenter / resolution;


    vec4 radiance = vec4(0.0);

    for(int i = 0; i < rayCount; i++) {

        float index = baseIndex + float(i);
        // Add 0.5 radians to avoid vertical angles
        float angleStep = (index + 0.5);
        float angle = angleStep * angleStepSize;
        vec2 rayDirection = vec2(cos(angle), -sin(angle));

        // Start in our decided starting location
        vec2 sampleUv = normalizedProbeCenter + (rayDirection * intervalStart * scale);
        // Keep track of how far we've gone
        float traveled = intervalStart;
        vec4 radDelta = vec4(0.0);

        // (Existing loop, but to reiterate, we're raymarching)
        for (int step = 1; step < maxSteps; step++) {
            // How far away is the nearest object?
            float dist = texture(distanceTexture, sampleUv).r;

            // Go the direction we're traveling
            sampleUv += rayDirection * dist * scale;

            if (outOfBounds(sampleUv)) break;

            // Read if our distance field tells us to!
            if (dist <= minStepSize) {
                // Accumulate radiance or shadow!
                vec4 colorSample = texture(ourTexture, sampleUv);
                radDelta += vec4(pow(colorSample.rgb, vec3(srgb)), colorSample.a);
                break;
            }

            // Stop if we've gone our interval length!
            traveled += dist;
            if (traveled >= intervalEnd) break;
        }


      // Only merge on non-opaque areas
      bool nonOpaque = radDelta.a == 0.0;
      if (rayCount == baseRayCount && nonOpaque) {
        // The spacing between probes
        float upperSpacing = sqrtBase;
        // Grid of probes
        vec2 upperSize = floor(resolution / upperSpacing);
        // Position of _this_ probe
        vec2 upperPosition = vec2(
          mod(index, sqrtBase), floor(index / upperSpacing)
        ) * upperSize;

        vec2 offset = (probeRelativePosition + 0.5) / sqrtBase;
        vec2 upperUv = (upperPosition + offset) / resolution;
        radDelta += texture(lastTexture, upperUv);
      }

        // Accumulate total radiance
        radiance += radDelta;
    }

    vec3 final = radiance.rgb * oneOverRayCount;
    vec3 correctSRGB = pow(final, vec3(srgb));

    return vec4(correctSRGB.rgb, 1.0);
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