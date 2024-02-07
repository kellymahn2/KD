#version 460 core

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout (binding = 0) uniform sampler2D depthTexture2D;
layout (binding = 1) uniform sampler2D depthTexture3D;

layout(binding = 0, rgba8) uniform readonly restrict image2D colorTexture2D;
layout(binding = 1, rgba8) uniform readonly restrict image2D colorTexture3D;
layout(binding = 2, rgba8) uniform writeonly restrict image2D outputTexture;

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    // Sample depth values
    float depth2D = texture(depthTexture2D, vec2(pixelCoords) / textureSize(depthTexture2D,0)).r;
    float depth3D = texture(depthTexture3D, vec2(pixelCoords) / textureSize(depthTexture3D,0)).r;

    // Compare depths and calculate blend factors
    float blendFactor2D = step(depth2D, depth3D);
    float blendFactor3D = 1.0 - blendFactor2D;

    // Sample colors
    vec4 color2D = imageLoad(colorTexture2D, pixelCoords);
    vec4 color3D = imageLoad(colorTexture3D, pixelCoords);

    // Blend colors based on depth comparison
    vec4 finalColor = color2D * blendFactor2D + color3D * blendFactor3D;

    // Store final color
    imageStore(outputTexture, pixelCoords, finalColor);
}
