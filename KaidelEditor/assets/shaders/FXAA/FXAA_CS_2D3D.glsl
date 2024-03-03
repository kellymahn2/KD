#version 430 core

layout (local_size_x = 32, local_size_y = 32,local_size_z = 1) in;

layout (binding = 0, rgba8) uniform image2D inputImage;
layout (binding = 1, rgba8) uniform image2D outputImage;

uniform ivec2 screenSize; // Screen width and height

// Function to compute luminance
float computeLuminance(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

void main() {
    ivec2 texelCoords = ivec2(gl_GlobalInvocationID.xy);

	ivec2 iSize = imageSize(inputImage);
    // Compute texel size
    vec2 texelSize = 1.0 / vec2(iSize.xy);

    // Sample texels for FXAA
    vec3 rgbNW = imageLoad(inputImage, texelCoords - ivec2(1, 1)).rgb;
    vec3 rgbNE = imageLoad(inputImage, texelCoords - ivec2(-1, 1)).rgb;
    vec3 rgbSW = imageLoad(inputImage, texelCoords - ivec2(1, -1)).rgb;
    vec3 rgbSE = imageLoad(inputImage, texelCoords - ivec2(-1, -1)).rgb;
    vec3 rgbM  = imageLoad(inputImage, texelCoords).rgb;

    // Compute luminance for each texel
    float lumaNW = computeLuminance(rgbNW);
    float lumaNE = computeLuminance(rgbNE);
    float lumaSW = computeLuminance(rgbSW);
    float lumaSE = computeLuminance(rgbSE);
    float lumaM  = computeLuminance(rgbM);

    // Compute edge direction
    float dirX = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    float dirY = ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    vec2 dir = normalize(vec2(dirX, dirY));

    // Compute length of the gradient
    float gradientLength = length(vec2(dirX, dirY));

    // Compute the length of the gradient to prevent aliasing
    float subpixelOffset = clamp(gradientLength / 4.0, 0.0, 1.0);
    dir *= subpixelOffset;

    // Apply FXAA
    vec3 rgbA = 0.5 * (
        imageLoad(inputImage, texelCoords - ivec2(dir)).rgb +
        imageLoad(inputImage, texelCoords + ivec2(dir)).rgb
    );
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        imageLoad(inputImage, texelCoords - ivec2(2.0 * dir)).rgb +
        imageLoad(inputImage, texelCoords + ivec2(2.0 * dir)).rgb
    );

    float lumaB = computeLuminance(rgbB);
    vec3 finalColor = (lumaB < min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)) || lumaB > max(max(lumaNW, lumaNE), max(lumaSW, lumaSE))) ? rgbA : rgbB;

    // Write the final color to the output texture
    imageStore(outputImage, texelCoords, vec4(finalColor, 1.0));
}
