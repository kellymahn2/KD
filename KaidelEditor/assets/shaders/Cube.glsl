// Basic Texture Shader

#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in int a_MaterialIndex;
layout(location = 3) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec3 u_CameraPosition;
};



struct VertexOutput
{
	vec3 FragPos;
	vec3 Normal;
};

layout (location = 0) out VertexOutput Output;
layout (location = 2) out flat int v_MaterialIndex;
layout (location = 3) out flat int v_EntityID;

void main()
{
	Output.Normal = a_Normal;
	Output.FragPos = a_Position;
	v_MaterialIndex = a_MaterialIndex;
	v_EntityID = a_EntityID;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 460 core

struct VertexOutput
{
	vec3 FragPos;
	vec3 Normal;
};

layout (location = 0) in VertexOutput Input;
layout (location = 2) in flat int v_MaterialIndex;
layout (location = 3) in flat int v_EntityID;

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

struct Material{
	float ColorX,ColorY,ColorZ,ColorW;
	float AmbientX,AmbientY,AmbientZ;
	float DiffuseX,DiffuseY,DiffuseZ;
	float SpecularX,SpecularY,SpecularZ;
	float Shininess;
};

struct Light {
    float PositionX,PositionY,PositionZ;
    float AmbientIntensityX,AmbientIntensityY,AmbientIntensityZ;
    float DiffuseIntensityX,DiffuseIntensityY,DiffuseIntensityZ;
    float SpecularIntensityX,SpecularIntensityY,SpecularIntensityZ;
};


struct DirectionalLight{
	float DirectionX,DirectionY,DirectionZ;
    float AmbientIntensityX,AmbientIntensityY,AmbientIntensityZ;
    float DiffuseIntensityX,DiffuseIntensityY,DiffuseIntensityZ;
    float SpecularIntensityX,SpecularIntensityY,SpecularIntensityZ;
};

layout(std430,binding = 3) buffer DirLight{
	DirectionalLight u_DirectionalLight;
}; 

layout(std430,binding = 1) buffer Materials{
	Material u_Materials[];
};

layout(std430,binding = 2) buffer Lights
{
	Light u_Lights[];
};




layout (binding = 0) uniform sampler2D u_Textures[32];


vec3 CalcLightAmbient(vec3 materialAmbient,vec3 ambientIntensity){
	return materialAmbient * ambientIntensity;
}

vec3 CalcLightDiffuse(vec3 materialDiffuse,vec3 norm,vec3 lightDir,vec3 diffuseIntensity){
	return materialDiffuse*max(dot(norm, lightDir), 0.0)*	diffuseIntensity;
}


vec4 ApplyLighting(Material material){

	vec3 totalAmbient = vec3(0.0);
	vec3 totalDiffuse = vec3(0.0);
	vec3 totalSpecular = vec3(0.0);

	vec4 materialColor = vec4(material.ColorX,material.ColorY,material.ColorZ,material.ColorW);
	vec3 materialAmbient = vec3(material.AmbientX,material.AmbientY,material.AmbientZ);
	vec3 materialDiffuse = vec3(material.DiffuseX,material.DiffuseY,material.DiffuseZ);
	vec3 materialSpecular = vec3(material.SpecularX,material.SpecularY,material.SpecularZ);
	vec3 norm = normalize(Input.Normal);

	//Directional Light
	{
		vec3 lightDir = normalize(-vec3(u_DirectionalLight.DirectionX,u_DirectionalLight.DirectionY,u_DirectionalLight.DirectionZ));
		
		vec3 ambientIntensity=vec3(u_DirectionalLight.AmbientIntensityX,u_DirectionalLight.AmbientIntensityY,u_DirectionalLight.AmbientIntensityZ);
		vec3 diffuseIntensity=vec3(u_DirectionalLight.DiffuseIntensityX,u_DirectionalLight.DiffuseIntensityY,u_DirectionalLight.DiffuseIntensityZ);
		vec3 specularIntensity=vec3(u_DirectionalLight.SpecularIntensityX,u_DirectionalLight.SpecularIntensityY,u_DirectionalLight.SpecularIntensityZ);


		totalAmbient += CalcLightAmbient(materialAmbient,ambientIntensity);

		//Diffuse
		totalDiffuse += CalcLightDiffuse(materialDiffuse,norm, lightDir,diffuseIntensity);

		//Specular
		vec3 viewDir = normalize(u_CameraPosition - Input.FragPos);
		vec3 reflectDir = reflect(-lightDir,norm);
		totalSpecular += materialSpecular*pow(max(dot(viewDir,reflectDir),0.0),32) * specularIntensity;
	}





	for(int i = 0;i < u_LightCount;++i){	

		Light light = u_Lights[i];

		vec3 lightPos = vec3(light.PositionX,light.PositionY,light.PositionZ);
		vec3 lightDir = normalize(lightPos-Input.FragPos);
		
		vec3 ambientIntensity = vec3(light.AmbientIntensityX,light.AmbientIntensityY,light.AmbientIntensityZ);
		vec3 diffuseIntensity = vec3(light.DiffuseIntensityX,light.DiffuseIntensityY,light.DiffuseIntensityZ);
		vec3 specularIntensity = vec3(light.SpecularIntensityX,light.SpecularIntensityY,light.SpecularIntensityZ);

		
		//Ambient
		totalAmbient += CalcLightAmbient(materialAmbient,ambientIntensity);

		//Diffuse
		totalDiffuse += CalcLightDiffuse(materialDiffuse,norm, lightDir,diffuseIntensity);

		//Specular
		vec3 viewDir = normalize(u_CameraPosition - Input.FragPos);
		vec3 reflectDir = reflect(-lightDir,norm);
		totalSpecular += materialSpecular*pow(max(dot(viewDir,reflectDir),0.0),material.Shininess) * specularIntensity;
	}

	vec3 result = totalAmbient + totalDiffuse + totalSpecular;
	materialColor.xyz =  result * materialColor.xyz;

	return materialColor;
}

void main()
{
	vec4 texColor = ApplyLighting(u_Materials[v_MaterialIndex]);

	if (texColor.a == 0.0)
		discard;




	o_Color = texColor;
	o_EntityID = v_EntityID;
}
