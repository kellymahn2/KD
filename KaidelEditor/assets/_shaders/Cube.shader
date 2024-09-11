#type vertex
#version 460 core
layout(location = 0)in vec3 a_Position;

layout(set = 0,binding = 0,std140)uniform SceneData{
	mat4 ViewProj;
} u_SceneData;

//layout(set = 2,binding = 0, std140)uniform Material{
//	vec4 Albedo;
//	float Roughness;
//	float Specular;
//	float Metallic;
//} u_Material;

layout(push_constant) uniform DrawData{
	mat4 Transform;
} u_DrawData;

void main(){
	gl_Position = u_SceneData.ViewProj * u_DrawData.Transform * vec4(a_Position,1.0);
}

#type fragment
#version 460 core

layout(location = 0)out vec4 o_Color;

//layout(set = 0,binding = 0, std140)uniform Material{
//	vec4 Albedo;
//	float Roughness;
//	float Specular;
//	float Metallic;
//} u_Material;

void main(){
	o_Color = vec4(1.0);
}
