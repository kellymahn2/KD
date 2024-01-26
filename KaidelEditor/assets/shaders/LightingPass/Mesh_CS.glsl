#version 460 core
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;



layout(binding = 0, rgba32f) restrict readonly uniform image2D inputPosition;
layout(binding = 1, rgba32f) restrict readonly uniform image2D inputNormal;
layout(binding = 2, r32i) restrict readonly uniform iimage2D inputIndex;
layout(binding = 3, rgba8) restrict readonly uniform image2D inputAlbedo;
layout(binding = 4, rgba8) restrict writeonly uniform image2D outputImage;


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
	int Diffuse;
	int Specular;
	float Shininess;
};



layout(std430,binding = 4) buffer SpotLights{

	SpotLight u_SpotLights[];
};

layout(std430,binding = 1) buffer Materials{
	Material u_Materials[];
};

layout(binding = 4) uniform sampler2DArray u_SpotLightDepthMaps;


float CalcShadowValue(vec3 position,vec3 normal){
	
	float totalShadow = 1.0;

	{
	
		for(int i = 0;i < u_SpotLightCount; ++i){
			vec4 posInLightSpace = u_SpotLights[i].LightViewProjection * vec4(position,1.0);
			vec3 projectedCoords = posInLightSpace.xyz / posInLightSpace.w;

			
			projectedCoords = projectedCoords * .5 + .5;


			if(projectedCoords.z > 1.0)
				continue;

			float currentDepth = projectedCoords.z;

			vec3 lightDir = normalize(u_SpotLights[i].Position.xyz - position);
			float bias = max(0.05 * (1.0 - dot(normal,lightDir)),0.005);

			float shadow = 0.0;

			vec2 texelSize = 1.0 / textureSize(u_SpotLightDepthMaps,0).xy;
			for(int j = -1;j<=1;++j){
				
				for(int k = -1;k<=1;++k){
					float pcfDepth = texture(u_SpotLightDepthMaps,vec3(projectedCoords.xy + vec2(j,k)*texelSize,float(i))).r;
					shadow += currentDepth  - bias > pcfDepth ? 1.0 : 0.0;
				}
			}
			shadow /= 9.0;

			totalShadow *= (1.0 - shadow);
		}
	}

	return totalShadow;
}


void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec3 position = imageLoad(inputPosition, texelCoord).xyz;
    vec3 normal = imageLoad(inputNormal, texelCoord).xyz;
    vec4 albedo = imageLoad(inputAlbedo, texelCoord);
	vec3 diffuse = albedo.rgb;
	float spec = albedo.a;
	

	vec3 totalAmbient = vec3(0.0);
	vec3 totalDiffuse = vec3(0.0);
	vec3 totalSpecular = vec3(0.0);
	int matIndex = int(imageLoad(inputIndex,texelCoord).r);
	

	vec4 materialColor = vec4(u_Materials[matIndex].ColorX,u_Materials[matIndex].ColorY,u_Materials[matIndex].ColorZ,u_Materials[matIndex].ColorW);
	float shininess = u_Materials[matIndex].Shininess;
	vec3 viewDir = normalize(u_CameraPosition - position);
	//SpotLight
	{
		for(int i =0;i < u_SpotLightCount;++i){
			vec3 lightPos = u_SpotLights[i].Position.xyz;
			vec3 lightDir = normalize(lightPos - position);

			vec3 lightDirection = u_SpotLights[i].Direction.xyz;
			vec3 lightAmbient = u_SpotLights[i].Ambient.xyz;


			float distance = length(lightPos - position);
			float attenuation = 1.0/(u_SpotLights[i].ConstantCoefficient+u_SpotLights[i].LinearCoefficient*distance+
			u_SpotLights[i].QuadraticCoefficient*(distance*distance));
			attenuation = 1.0;

			//Ambient
			totalAmbient += lightAmbient * diffuse * attenuation;
			
			float theta = dot(lightDir,normalize(-lightDirection));
			if(theta > u_SpotLights[i].CutOffAngle){
				vec3 lightDiffuse = u_SpotLights[i].Diffuse.rgb;
				vec3 lightSpecular = u_SpotLights[i].Specular.rgb;

				//Diffuse
				totalDiffuse += diffuse * max(dot(normal, lightDir), 0.0) * lightDiffuse * attenuation;

				//Specular
				vec3 reflectDir = reflect(-lightDir,normal);
				totalSpecular += lightSpecular * pow(max(dot(viewDir,reflectDir),0.0),shininess) * spec * attenuation;
			}
		
		}
	
	}

	float shadow =CalcShadowValue(position,normal);

	//float shadow = 1.0;
	vec3 res = totalAmbient + (shadow)*(totalDiffuse + totalSpecular);




    vec4 resultColor = vec4(materialColor.xyz * (res),1.0);
    imageStore(outputImage, texelCoord, resultColor);
	

}
