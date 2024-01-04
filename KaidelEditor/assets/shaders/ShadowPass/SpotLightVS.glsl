#version 460 core

layout(location = 0) in vec3 a_Position;


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

void main(){
	gl_Position = u_SpotLights[u_LightIndex].LightViewProjection* vec4(a_Position, 1.0);
}
