#version 460 core
layout(location = 0)in vec3 a_Position;
layout(location = 1)in vec3 a_Normal;
layout(location = 2)in vec2 a_TexCoords;

layout(location = 3) in mat4 a_Transform;
layout(location = 7) in mat3 a_NormalTransform;
layout(location = 10) in int a_MaterialID;


struct VSOutput{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
};


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec3 u_CameraPosition;
};

layout(location = 0)out VSOutput Output;
layout(location = 3)out flat int u_MatIndex;



void main(){
	mat4 transform = a_Transform;
	mat3 normal = a_NormalTransform;
	int matID = a_MaterialID;

	vec4 WorldPos = transform *vec4(a_Position,1.0);
	Output.Position = WorldPos.xyz;
	Output.TexCoords = a_TexCoords;
	Output.Normal = (normal * a_Normal);
	Output.Normal = normalize(Output.Normal);
	u_MatIndex = matID;
	gl_Position = u_ViewProjection * WorldPos;
}
