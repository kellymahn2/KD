// Compute Shader for Composite
#version 460 core

layout (local_size_x = 16, local_size_y = 16) in;

layout(location = 0,rgba8) uniform image2D a_Color;
layout(location = 1,rgba8) uniform image2D a_Ambient;
layout(location = 2,rgba8) uniform image2D a_Diffuse;
layout(location = 3,rgba8) uniform image2D a_Specular;


layout(binding = 4, rgba8) uniform image2D outputImage;
layout(binding = 5) uniform sampler2DArray u_DepthMaps;
void main() {
    uvec2 pixelCoords = gl_GlobalInvocationID.xy;
}
