#version 460 core

layout(location = 0) in vec3 a_Position;


uniform int u_LightIndex;

struct SpotLight {
	mat4 LightViewProjection;
	float PositionX,PositionY,PositionZ;
	float DirectionX,DirectionY,DirectionZ;
	float AmbientX,AmbientY,AmbientZ;
	float DiffuseX,DiffuseY,DiffuseZ;
	float SpecularX,SpecularY,SpecularZ;
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
