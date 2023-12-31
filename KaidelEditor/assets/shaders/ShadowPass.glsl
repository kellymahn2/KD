// Basic Texture Shader

#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in int a_MaterialIndex;
layout(location = 4) in int a_EntityID;


layout(std140, binding = 1) uniform Light
{
	mat4 u_ViewProjection;
	vec3 u_LightPosition;
};

struct VertexOutput
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoord;
};


void main(){
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 460 core


void main()
{
	
}
