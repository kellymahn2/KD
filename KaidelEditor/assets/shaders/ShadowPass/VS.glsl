#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in int a_MaterialIndex;
layout(location = 4) in int a_EntityID;


uniform int u_LightIndex;

struct DirectionalLight{
	mat4 LightViewProjection;
	float DirectionX,DirectionY,DirectionZ;
    float AmbientIntensityX,AmbientIntensityY,AmbientIntensityZ;
    float DiffuseIntensityX,DiffuseIntensityY,DiffuseIntensityZ;
    float SpecularIntensityX,SpecularIntensityY,SpecularIntensityZ;
};

struct PointLight {
	mat4 LightViewProjection;
	float PositionX,PositionY,PositionZ;
	float AmbientX,AmbientY,AmbientZ;
	float DiffuseX,DiffuseY,DiffuseZ;
	float SpecularX,SpecularY,SpecularZ;

	float ConstantCoefficient;
	float LinearCoefficient;
	float QuadraticCoefficient;
};


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


layout(std430,binding = 3) buffer DirLight{
	DirectionalLight u_DirectionalLight;
}; 


layout(std430,binding = 4) buffer SpotLights{

	SpotLight u_SpotLights[];
};

void main(){
	gl_Position = u_SpotLights[u_LightIndex].LightViewProjection* vec4(a_Position, 1.0);
}
