#version 460 core
layout(location = 0)in vec3 a_Position;
layout(location = 1)in vec3 a_Normal;
layout(location = 2)in vec2 a_TexCoords;


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

struct MeshDrawData {
	mat4 Transform;
	mat3 NormalTransform;
	int MaterialID;
};

layout(std140,binding = 5) buffer DrawData{
	MeshDrawData u_DrawData[];
};

layout(location = 0)out VSOutput Output;
layout(location = 3)out flat int u_MatIndex;


void main(){
	vec4 WorldPos = u_DrawData[gl_InstanceID].Transform*vec4(a_Position,1.0);
	Output.Position = WorldPos.xyz;
	Output.TexCoords = a_TexCoords;
	Output.Normal = (u_DrawData[gl_InstanceID].NormalTransform * a_Normal);
	Output.Normal = normalize(Output.Normal);
	u_MatIndex = u_DrawData[gl_InstanceID].MaterialID;
	gl_Position = u_ViewProjection * WorldPos;
}
