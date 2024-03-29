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

struct Material{
	float ColorX,ColorY,ColorZ,ColorW;
	uint Diffuse;
	uint Specular;
	float Shininess;
};



layout(std430,binding = 4) buffer SpotLights{

	SpotLight u_SpotLights[];
};

layout(std430,binding = 1) buffer Materials{
	Material u_Materials[];
};

layout(binding = 4) uniform sampler2DArray u_SpotLightDepthMaps;


layout(std140, binding = 5) uniform Settings
{
	uint u_AntiAiliasing;
	uint u_MSAASampleCount;
};
