#version 460 core

layout(location = 0)in vec3 a_Position;
layout(location = 1)in vec3 a_Normal;
layout(location = 2)in vec2 a_TexCoords;
layout(location = 3)in int a_MaterialIndex;


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec3 u_CameraPosition;
};

struct VSOutput
{
	vec2 TexCoords;
};

layout(location = 0)out VSOutput Output;
layout(location = 1)out flat int v_MaterialIndex;


void main(){
    Output.TexCoords = a_TexCoords;
    v_MaterialIndex = a_MaterialIndex;
   	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}