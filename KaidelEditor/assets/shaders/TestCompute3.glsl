#version 460 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D imgOutput;

layout(std140, binding = 0) uniform time
{
	float Time;
};

layout(std430, binding = 0) buffer a_UAV1
{
    float data_SSBO[];
};


void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    imageStore(imgOutput, texelCoord, vec4(float(data_SSBO[0]),0.0f,0.0f,1.0));
}
