#version 460 core

struct VertexOutput
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoord;

};

layout (location = 0) in VertexOutput Input;
layout(location = 3) in flat int MaterialIndex;


layout(location = 0)out vec4 o_Ambient;
layout(location = 1)out vec4 o_Diffuse;
layout(location = 2)out vec4 o_Specular;





layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec3 u_CameraPosition;
};


layout(std140, binding = 1) uniform LightingData
{
	int u_PointLightCount;
	int u_SpotLightCount;
};
layout(binding = 0) uniform sampler2DArray u_MaterialTextures;
struct Material{
	float ColorX,ColorY,ColorZ,ColorW;
	int Diffuse;
	int Specular;
	float Shininess;
};



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



layout(std430,binding = 1) buffer Materials{
	Material u_Materials[];
};

layout(std430,binding = 2) buffer PointLights
{
	PointLight u_PointLights[];
};

layout(std430,binding = 3) buffer DirLight{
	DirectionalLight u_DirectionalLight;
}; 


layout(std430,binding = 4) buffer SpotLights{

	SpotLight u_SpotLights[];
};

struct LightCalcResult{
	vec3 TotalAmbient;
	vec3 TotalDiffuse;
	vec3 TotalSpecular;
};



vec3 CalcLightAmbient(vec3 materialAmbient,vec3 ambientIntensity){
	return materialAmbient * ambientIntensity;
}

vec3 CalcLightDiffuse(vec3 materialDiffuse,vec3 norm,vec3 lightDir,vec3 diffuseIntensity){
	return materialDiffuse*max(dot(norm, lightDir), 0.0)*	diffuseIntensity;
}

LightCalcResult ApplyLighting(Material material){

	vec3 totalAmbient = vec3(0.0);
	vec3 totalDiffuse = vec3(0.0);
	vec3 totalSpecular = vec3(0.0);

	vec3 materialAmbient = texture(u_MaterialTextures,vec3(Input.TexCoord,material.Diffuse)).xyz;
	vec3 materialDiffuse = materialAmbient;
	vec3 materialSpecular = texture(u_MaterialTextures,vec3(Input.TexCoord,material.Specular)).xyz;
	vec3 norm = normalize(Input.Normal);
	vec3 viewDir = normalize(u_CameraPosition - Input.FragPos);

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
		vec3 halfwayDir = normalize(lightDir + viewDir);
		totalSpecular += materialSpecular*pow(max(dot(halfwayDir,norm),0.0),material.Shininess) * specularIntensity;
	}


	//Point Lights
	{
		for(int i =0;i<u_PointLightCount;++i){
			vec3 lightAmbient=vec3(u_PointLights[i].AmbientX,u_PointLights[i].AmbientY,u_PointLights[i].AmbientZ);
			vec3 lightDiffuse=vec3(u_PointLights[i].DiffuseX,u_PointLights[i].DiffuseY,u_PointLights[i].DiffuseZ);
			vec3 lightSpecular=vec3(u_PointLights[i].SpecularX,u_PointLights[i].SpecularY,u_PointLights[i].SpecularZ);
		

			vec3 lightPos = vec3(u_PointLights[i].PositionX,u_PointLights[i].PositionY,u_PointLights[i].PositionZ);
			vec3 lightDir = normalize(lightPos-Input.FragPos);
			float distance = length(lightPos - Input.FragPos);
			float attenuation = 1.0/(u_PointLights[i].ConstantCoefficient+u_PointLights[i].LinearCoefficient*distance+
			u_PointLights[i].QuadraticCoefficient*(distance*distance));


			//Ambient
			totalAmbient += CalcLightAmbient(materialAmbient,lightAmbient)*attenuation;


			//Diffuse
			totalDiffuse += CalcLightDiffuse(materialDiffuse,norm,lightDir,lightDiffuse) * attenuation;
			
			//Specular
			vec3 halfwayDir = normalize(lightDir + viewDir);
			totalSpecular += lightSpecular * pow(max(dot(halfwayDir,norm), 0.0), material.Shininess)*materialSpecular * attenuation;

		}
	
	}


	//SpotLight
	{
	
		for(int i =0;i<u_SpotLightCount;++i){
		
			vec3 lightPos = vec3(u_SpotLights[i].PositionX,u_SpotLights[i].PositionY,u_SpotLights[i].PositionZ);
			vec3 lightDir = normalize(lightPos - Input.FragPos);
		
		
			vec3 lightDirection = vec3(u_SpotLights[i].DirectionX,u_SpotLights[i].DirectionY,u_SpotLights[i].DirectionZ);
			vec3 lightAmbient=vec3(u_SpotLights[i].AmbientX,u_SpotLights[i].AmbientY,u_SpotLights[i].AmbientZ);
			
			
			float distance = length(lightPos - Input.FragPos);
			float attenuation = 1.0/(u_SpotLights[i].ConstantCoefficient+u_SpotLights[i].LinearCoefficient*distance+
			u_SpotLights[i].QuadraticCoefficient*(distance*distance));
			
			
			//Ambient
			totalAmbient += CalcLightAmbient(materialAmbient,lightAmbient)*attenuation;
			float theta = dot(lightDir,normalize(-lightDirection));
			if(theta > u_SpotLights[i].CutOffAngle){
				vec3 lightDiffuse=vec3(u_SpotLights[i].DiffuseX,u_SpotLights[i].DiffuseY,u_SpotLights[i].DiffuseZ);
				vec3 lightSpecular=vec3(u_SpotLights[i].SpecularX,u_SpotLights[i].SpecularY,u_SpotLights[i].SpecularZ);
				
		
		
		
				//Diffuse
				totalDiffuse += CalcLightDiffuse(materialDiffuse,norm,lightDir,lightDiffuse)*attenuation;
		
				//Specular
				vec3 halfwayDir = normalize(lightDir + viewDir);
				totalSpecular += lightSpecular * pow(max(dot(norm,halfwayDir),0.0),material.Shininess)*materialSpecular * attenuation;
			
			}
		}
	
	}
	LightCalcResult res;
	res.TotalAmbient = totalAmbient;
	res.TotalDiffuse = totalDiffuse;
	res.TotalSpecular = totalSpecular;
	return res;
}

void main(){

	LightCalcResult res = ApplyLighting(u_Materials[MaterialIndex]);

	o_Ambient = vec4(res.TotalAmbient,1.0);
	o_Diffuse = vec4(res.TotalDiffuse,1.0);
	o_Specular = vec4(res.TotalSpecular,1.0);

}

