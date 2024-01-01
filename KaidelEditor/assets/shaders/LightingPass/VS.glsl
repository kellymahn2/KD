#version 460 core
layout(location = 0)in vec3 a_Position;
layout(location = 1)in vec3 a_Normal;
layout(location = 2)in vec2 a_TexCoord;
layout(location = 3)in int a_MaterialIndex;


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
layout(location = 3) out flat int MaterialIndex;

void main(){
	Output.FragPos = a_Position;
	Output.Normal = a_Normal;
	Output.TexCoord = a_TexCoord;
	MaterialIndex = a_MaterialIndex;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
