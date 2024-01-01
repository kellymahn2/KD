#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in int a_MaterialIndex;
layout(location = 4) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec3 u_CameraPosition;
};



struct VertexOutput
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoord;
};

layout (location = 0) out VertexOutput Output;
layout (location = 3) out flat int v_MaterialIndex;
layout (location = 4) out flat int v_EntityID;

void main()
{
	Output.Normal = a_Normal;
	Output.FragPos = a_Position;
	Output.TexCoord = a_TexCoords;
	v_MaterialIndex = a_MaterialIndex;
	v_EntityID = a_EntityID;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
