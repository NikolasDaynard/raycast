// #version 330 core
// out vec4 FragColor;
  
// precision highp float;
// in vec3 ourColor;
// in vec2 TexCoord;

// uniform sampler2D distanceTexture;
// uniform sampler2D ourTexture; // input
// uniform sampler2D lastTexture;
// uniform int rayCount;
// uniform vec2 resolution;
// uniform int baseRayCount;

// bool outOfBounds(vec2 uv) {
//   return uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;
// }

// float rand(vec2 n) { 
// 	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
// }

// vec4 raymarch() {
//     vec4 light = texture(ourTexture, TexCoord);

//     if (light.a > 0.1) {
//         return light;
//     }
//     const int maxSteps = 40;
//     const float PI = 3.14159265;
//     const float TAU = 2.0 * PI;
//     const float minStepSize = .001;
//     const float srgb = 1.0;//.8;
//     vec2 scale = min(resolution.x, resolution.y) / resolution;
//     float oneOverRayCount = 1.0 / float(rayCount);
//     float tauOverRayCount = TAU * oneOverRayCount;
//     float angleStepSize = TAU * oneOverRayCount;

//     vec2 coord = floor(TexCoord * resolution);

//     bool isLastLayer = rayCount == baseRayCount;
//     vec2 effectiveUv = isLastLayer ? TexCoord : floor(coord / 2.0) * 2.0 / resolution;

//     float partial = 0.125;

//     float intervalStart = rayCount == baseRayCount ? 0.0 : partial;
//     float intervalEnd = rayCount == baseRayCount ? partial : sqrt(2.0);


//     vec4 radiance = vec4(0.0);

//     for(int i = 0; i < rayCount; i++) {

//         float index = float(i);
//         // Add 0.5 radians to avoid vertical angles
//         float angleStep = (index + 0.5);
//         float angle = angleStep * angleStepSize;
//         vec2 rayDirection = vec2(cos(angle), -sin(angle));

//         // Start in our decided starting location
//         vec2 sampleUv = effectiveUv + (rayDirection * intervalStart * scale);
//         // Keep track of how far we've gone
//         float traveled = intervalStart;
//         vec4 radDelta = vec4(0.0);

//         // (Existing loop, but to reiterate, we're raymarching)
//         for (int step = 1; step < maxSteps; step++) {
//             // How far away is the nearest object?
//             float dist = texture(distanceTexture, sampleUv).r;

//             // Go the direction we're traveling
//             sampleUv += rayDirection * dist * scale;

//             if (outOfBounds(sampleUv)) break;

//             // Read if our distance field tells us to!
//             if (dist < minStepSize) {
//                 // Accumulate radiance or shadow!
//                 vec4 colorSample = texture(ourTexture, sampleUv);
//                 radDelta += vec4(pow(colorSample.rgb, vec3(srgb)), 1.0);
//                 break;
//             }

//             // Stop if we've gone our interval length!
//             traveled += dist;
//             if (traveled >= intervalEnd) break;
//         }


//       // Only merge on non-opaque areas
//       if (rayCount == baseRayCount && radDelta.a < .1) {
//         vec4 upperSample = texture(lastTexture, TexCoord);

//         radDelta += vec4(pow(upperSample.rgb, vec3(srgb)), upperSample.a);
//       }

//         // Accumulate total radiance
//         radiance += radDelta;
//     }

//     vec3 final = radiance.rgb * oneOverRayCount;
//     vec3 correctSRGB = pow(final, vec3(srgb));

//     return vec4(correctSRGB, 1.0);
// }


// void main()
// {
//     // FragColor = (texture(ourTexture, TexCoord) + texture(ourTexture, TexCoord + vec2(0.1, 0.1))) / 2.0;
//     // FragColor = texture(ourTexture, TexCoord) + 0.1;
//     // FragColor = texture(distanceTexture, TexCoord);
//     // FragColor = vec4(TexCoord.x, TexCoord.y, 0.2, 1.0);
//     FragColor = vec4(raymarch().rgb, 1.0);
// }

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
        return light;
    }
    const int maxSteps = 40;
    const float PI = 3.14159265;
    const float TAU = 2.0 * PI;
    const float minStepSize = .001;
    const float srgb = 1.0;//.8;
    vec2 scale = min(resolution.x, resolution.y) / resolution;
    float oneOverRayCount = 1.0 / float(rayCount);
    float tauOverRayCount = TAU * oneOverRayCount;

    vec2 coord = floor(TexCoord * resolution);
    // A handy term we use in other calculations
    float sqrtBase = sqrt(float(baseRayCount));
    // The width / space between probes
    // If our `baseRayCount` is 16, this is 4 on the upper cascade or 1 on the lower.
    float spacing = rayCount == baseRayCount ? 1.0 : sqrtBase;
    // Calculate the number of probes per x/y dimension
    vec2 size = floor(resolution / spacing);
    // Calculate which probe we're processing this pass
    vec2 probeRelativePosition = mod(coord, size);
    // Calculate which group of rays we're processing this pass
    vec2 rayPos = floor(coord / size);
    // Calculate the index of the set of rays we're processing
    float baseIndex = float(baseRayCount) * (rayPos.x + (spacing * rayPos.y));
    // Calculate the size of our angle step
    float angleStepSize = TAU / float(rayCount);
    // Find the center of the probe we're processing
    vec2 probeCenter = (probeRelativePosition + 0.5) * spacing;

    bool isLastLayer = rayCount == baseRayCount;
    vec2 effectiveUv = isLastLayer ? TexCoord : floor(coord / 2.0) * 2.0 / resolution;

    float partial = 0.125;

    float intervalStart = rayCount == baseRayCount ? 0.0 : partial; // partial = dist to split
    float intervalEnd = rayCount == baseRayCount ? partial : sqrt(2.0);


    vec4 radiance = vec4(0.0);

        // Shoot rays in "rayCount" directions, equally spaced
    for (int i = 0; i < baseRayCount; i++) {
        float index = baseIndex + float(i);
        float angleStep = index + 0.5;
        float angle = angleStep * angleStepSize;
        vec2 rayDirection = vec2(cos(angle), -sin(angle));

        // Start in our decided starting location
        vec2 sampleUv = effectiveUv + (rayDirection * intervalStart * scale);
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
            if (dist < minStepSize) {
                // Accumulate radiance or shadow!
                vec4 colorSample = texture(ourTexture, sampleUv);
                radDelta += vec4(pow(colorSample.rgb, vec3(srgb)), 1.0);
                break;
            }

            // Stop if we've gone our interval length!
            traveled += dist;
            if (traveled >= intervalEnd) break;
        }


      // Only merge on non-opaque areas
      if (rayCount == baseRayCount && radDelta.a == 0.0) {
        float upperSpacing = sqrtBase;
        vec2 upperSize = floor(resolution / upperSpacing);
        vec2 upperPosition = vec2(
          mod(index, sqrtBase), floor(index / upperSpacing)
        ) * upperSize;

        vec2 offset = (probeRelativePosition + 0.5) / upperSpacing;

        vec4 upperSample = texture(
          lastTexture,
          (upperPosition + offset) / resolution
        );

        radDelta += vec4(upperSample.rgb, upperSample.a);
      }

        // Accumulate total radiance
        radiance += radDelta;
    }

    vec3 final = radiance.rgb * oneOverRayCount;
    vec3 correctSRGB = pow(final, vec3(srgb));

    return vec4(correctSRGB, 1.0);
}


void main()
{
    // FragColor = (texture(ourTexture, TexCoord) + texture(ourTexture, TexCoord + vec2(0.1, 0.1))) / 2.0;
    // FragColor = texture(ourTexture, TexCoord) + 0.1;
    // FragColor = texture(distanceTexture, TexCoord);
    // FragColor = vec4(TexCoord.x, TexCoord.y, 0.2, 1.0);
    FragColor = vec4(raymarch().rgb, 1.0);
}
