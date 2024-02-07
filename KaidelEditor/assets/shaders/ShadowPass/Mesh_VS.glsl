#version 460 core
layout(location = 0)in vec3 a_Position;
layout(location = 1)in vec3 a_Normal;
layout(location = 2)in vec2 a_TexCoords;

layout(location = 3) in mat4 a_Transform;
layout(location = 7) in mat3 a_NormalTransform;
layout(location = 10) in int a_MaterialID;

uniform int u_LightIndex;

struct SpotLight {
	mat4 LightViewProjection;
	vec4 Position;
	vec4 Direction;
	vec4 Ambient;
	vec4 Diffuse;
	vec4 Specular;

	float CutOffAngle;
	float ConstantCoefficient;
	float LinearCoefficient;
	float QuadraticCoefficient;
};

layout(std430,binding = 4) buffer SpotLights{

	SpotLight u_SpotLights[];
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




void main(){

	gl_Position = u_SpotLights[u_LightIndex].LightViewProjection * a_Transform * vec4(a_Position,1.0);
	
}
