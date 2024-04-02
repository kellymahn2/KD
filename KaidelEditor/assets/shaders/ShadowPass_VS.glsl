#version 460 core
layout(location = 0)in vec3 a_Position;
layout(location = 1)in vec3 a_Normal;
layout(location = 2)in vec2 a_TexCoord;
layout(location = 3)in mat4 a_Transform;

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

layout(std430,binding = 0) buffer SpotLights{

	SpotLight u_SpotLights[];
};

layout(binding = 0) uniform sampler2DArray u_SpotLightDepthMaps;


void main(){
	gl_Position = u_SpotLights[u_LightIndex].LightViewProjection * a_Transform * vec4(a_Position,1.0);
}
