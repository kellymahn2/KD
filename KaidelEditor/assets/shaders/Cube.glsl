// Basic Texture Shader

#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec3 a_Normal;	
layout(location = 3) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec3 u_CameraPosition;
};



struct VertexOutput
{
	vec4 Color;
	vec3 FragPos;
	vec3 Normal;
};

layout (location = 0) out VertexOutput Output;
layout (location = 3) out flat int v_EntityID;

void main()
{
	Output.Color = a_Color;
	Output.Normal = a_Normal;
	Output.FragPos = a_Position;
	v_EntityID = a_EntityID;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 460 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec3 u_CameraPosition;
};
layout(std140, binding = 1) uniform CountInfo
{
	int u_LightCount;
	int u_MaterialCount;
};
struct VertexOutput
{
	vec4 Color;
	vec3 FragPos;
	vec3 Normal;
};


struct Light {
    float ColorX,ColorY,ColorZ;
    float PositionX,PositionY,PositionZ;
    float AmbientIntensity;
    float DiffuseIntensity;
    float SpecularIntensity;
};

layout(std430,binding = 2) buffer Lights
{
	Light u_Lights[];
};


layout (location = 0) in VertexOutput Input;
layout (location = 3) in flat int v_EntityID;

layout (binding = 0) uniform sampler2D u_Textures[32];

vec4 ApplyLighting(vec4 objectColor){

	vec3 totalAmbient = vec3(0.0);
	vec3 totalDiffuse = vec3(0.0);
	vec3 totalSpecular = vec3(0.0);
	for(int i = 0;i < u_LightCount;++i){	
		vec3 lightColor = vec3(u_Lights[i].ColorX,u_Lights[i].ColorY,u_Lights[i].ColorZ);
		vec3 lightPos = vec3(u_Lights[i].PositionX,u_Lights[i].PositionY,u_Lights[i].PositionZ);
		vec3 norm = normalize(Input.Normal);
		vec3 lightDir = normalize(lightPos-Input.FragPos);
		
		
		//Ambient
		totalAmbient += u_Lights[i].AmbientIntensity * lightColor;

		//Diffuse
		totalDiffuse += u_Lights[i].DiffuseIntensity*max(dot(norm, lightDir), 0.0)* lightColor;

		//Specular
		vec3 viewDir = normalize(u_CameraPosition - Input.FragPos);
		vec3 reflectDir = reflect(-lightDir,norm);
		totalSpecular += u_Lights[i].SpecularIntensity*pow(max(dot(viewDir,reflectDir),0.0),32) * lightColor;
	}

	vec3 result = totalAmbient + totalDiffuse + totalSpecular;
	vec3 col =  result * vec3(objectColor.x,objectColor.y,objectColor.z);
	objectColor.xyz = col;

	return objectColor;
}

void main()
{
	vec4 texColor = Input.Color;
	if (texColor.a == 0.0)
		discard;



	texColor = ApplyLighting(texColor);

	o_Color = texColor;
	o_EntityID = v_EntityID;
}
