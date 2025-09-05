#version 460 core
#ifdef vertex
layout(location = 0) in vec3 a_Position;

layout(push_constant) uniform PushConstants {
    mat4 ViewProjection;
    uint InstanceOffset;
};

layout(set = 0, binding = 0) buffer _InstanceData
{
	mat4 InstanceTransform[];
};



void main() {
    mat4 transform = InstanceTransform[gl_InstanceIndex + InstanceOffset];
    // Transform the vertex position to world space
    gl_Position =  ViewProjection * transform * vec4(a_Position, 1.0);
}
#endif
