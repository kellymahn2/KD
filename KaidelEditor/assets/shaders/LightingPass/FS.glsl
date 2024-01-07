#version 460 core

struct VertexOutput
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoord;

};

layout (location = 0) in VertexOutput Input;
layout(location = 3) in flat int MaterialIndex;


layout(location = 0)out vec4 o_Color;


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
layout(binding = 4) uniform sampler2DArray u_SpotLightDepthMaps;
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



float CalcShadowValue(vec3 normal){

	float totalShadow = 0.0;
	vec2 texelSize = 1.0 / (textureSize(u_SpotLightDepthMaps,0).xy);
    for(int i =0;i<u_SpotLightCount;++i){
		vec3 lightPos = u_SpotLights[i].Position.xyz;
		vec3 lightDir = normalize(lightPos - Input.FragPos);

        vec4 lightSpacePos = u_SpotLights[i].LightViewProjection * vec4(Input.FragPos,1.0);
		vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
		projCoords = projCoords * 0.5 + 0.5;
		if(projCoords.z > 1.0)
			continue;

		float currentDepth = projCoords.z;

		float bias = max(0.05*(1.0 - dot(normal,lightDir)),0.0);

		float shadow = 0.0;
		
		for(int x = -1;x<=1;++x){
			for(int y = -1;y <= 1;++y){
				float pcfDepth = texture(u_SpotLightDepthMaps,vec3(projCoords.xy + vec2(x,y)* texelSize,i)).r;
				shadow += currentDepth - bias > pcfDepth? 1.0 : 0.0;
			}
		}
		shadow /= 9.0;
		//for(int x = -4;x<=4;++x){
		//	for(int y = -4;y <= 4;++y){
		//		float pcfDepth = texture(u_SpotLightDepthMaps,vec3(projCoords.xy + vec2(x,y)* texelSize,i)).r;
		//		shadow += currentDepth - bias > pcfDepth? 1.0 : 0.0;
		//	}
		//}
		//shadow /= 81.0;
		totalShadow += shadow;
    }
	totalShadow/= u_SpotLightCount;
	return totalShadow;
}

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
		
			vec3 lightPos = u_SpotLights[i].Position.xyz;
			vec3 lightDir = normalize(lightPos - Input.FragPos);
		
		
			vec3 lightDirection = u_SpotLights[i].Direction.xyz;
			vec3 lightAmbient= u_SpotLights[i].Ambient.xyz;
			
			
			float distance = length(lightPos - Input.FragPos);
			float attenuation = 1.0/(u_SpotLights[i].ConstantCoefficient+u_SpotLights[i].LinearCoefficient*distance+
			u_SpotLights[i].QuadraticCoefficient*(distance*distance));
			
			
			//Ambient
			totalAmbient += CalcLightAmbient(materialAmbient,lightAmbient)*attenuation;
			float theta = dot(lightDir,normalize(-lightDirection));
			if(theta > u_SpotLights[i].CutOffAngle){
				vec3 lightDiffuse= u_SpotLights[i].Diffuse.xyz;
				vec3 lightSpecular= u_SpotLights[i].Specular.xyz;
				
		
		
		
				//Diffuse
				totalDiffuse += CalcLightDiffuse(materialDiffuse,norm,lightDir,lightDiffuse)*attenuation;
		
				//Specular
				vec3 halfwayDir = normalize(lightDir + viewDir);
				totalSpecular += lightSpecular * pow(max(dot(norm,halfwayDir),0.0),material.Shininess)*materialSpecular * attenuation;
			
			}
		}
	
	}
	vec4 res = vec4((totalAmbient + (1.0 - CalcShadowValue(norm))*(totalDiffuse + totalSpecular)),1.0)*materialColor;
	return res;
}

void main(){

	vec4 res = ApplyLighting(u_Materials[MaterialIndex]);
    o_Color = res;
}

