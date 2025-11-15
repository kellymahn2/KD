#version 460 core
#ifdef vertex

layout(location = 0) in vec3 a_Position;

layout(push_constant) uniform PushConstants
{
    mat4 ViewProjection;
    mat4 Model;
    vec4 Color;
};

void main()
{
    gl_Position = ViewProjection * Model * vec4(a_Position, 1.0);
}
#endif

#ifdef fragment

layout(location = 0) out vec4 o_Color;

layout(push_constant) uniform PushConstants
{
    mat4 ViewProjection;
    mat4 Model;
    vec4 Color;
};

void main()
{
    o_Color = Color;
}
#endif
