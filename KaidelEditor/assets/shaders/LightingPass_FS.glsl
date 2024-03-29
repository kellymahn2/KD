#version 460 core



layout(location = 0) out vec4 o_FragColor;

#include "FullScreenQuadOutput.glsl"


layout(location = 0) in FragmentData v_FragmentData;

layout(binding = 0) uniform sampler2DMS u_InputPosition;
layout(binding = 1) uniform sampler2DMS u_InputNormal;
layout(binding = 2) uniform sampler2DMS u_InputIndex;
layout(binding = 3) uniform sampler2DMS u_InputAlbedo;

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

#include "Core/Lighting.glsl"

float CalcShadowValue(vec3 position,vec3 normal){
	float totalShadow = 1.0;

	{
	
		for(int i = 0;i < u_SpotLightCount; ++i){
			vec4 posInLightSpace = u_SpotLights[i].LightViewProjection * vec4(position,1.0);
			vec3 projectedCoords = posInLightSpace.xyz / posInLightSpace.w;

			
			projectedCoords = projectedCoords * .5 + .5;


			if(projectedCoords.z > 1.0)
				continue;

			float closestDepth = texture(u_SpotLightDepthMaps,vec3(projectedCoords.xy,float(i))).r;
			float currentDepth = projectedCoords.z;

			vec3 lightDir = normalize(u_SpotLights[i].Position.xyz - position);
			float bias = max(0.05 * (1.0 - dot(normal,lightDir)),0.005);

			float shadow = 0.0;

			vec2 texelSize = 1.0 / textureSize(u_SpotLightDepthMaps,0).xy;
			int size = 1;
			for(int j = -size;j<=size;++j){
				
				for(int k = -size;k<=size;++k){
					float pcfDepth = texture(u_SpotLightDepthMaps,vec3(projectedCoords.xy + vec2(j,k)*texelSize,float(i))).r;
					shadow += currentDepth  - bias > pcfDepth ? 1.0 : 0.0;
				}
			}
			shadow /= (2*size+1) * (2*size+1);

			totalShadow *= (1.0-shadow);
		}
	}

	return totalShadow;
}


void main() {

	vec2 texelCoord = v_FragmentData.ScreenPosition;

	vec4 fragCol = vec4(0.0);

	for(int currentSample = 0;currentSample<u_MSAASampleCount;++currentSample){
		
		vec3 position = texelFetch(u_InputPosition,ivec2(texelCoord),currentSample).rgb;
		vec3 normal = texelFetch(u_InputNormal,ivec2(texelCoord),currentSample).rgb;
		vec4 albedo = texelFetch(u_InputAlbedo,ivec2(texelCoord),currentSample);

		vec3 diffuse = albedo.rgb;
		float spec = albedo.a;


		vec3 totalAmbient = vec3(0.0);
		vec3 totalDiffuse = vec3(0.0);
		vec3 totalSpecular = vec3(0.0);
		int matIndex = int(texelFetch(u_InputIndex,ivec2(texelCoord),currentSample).r);
		vec4 materialColor = vec4(u_Materials[matIndex].ColorX,u_Materials[matIndex].ColorY,u_Materials[matIndex].ColorZ,u_Materials[matIndex].ColorW);
		float shininess = u_Materials[matIndex].Shininess;


		vec3 viewDir = normalize(u_CameraPosition - position);

		//SpotLight
		{
			for(int i =0;i < u_SpotLightCount;++i){

				LightingCalcResult res = CalculateFragmentLighting(u_SpotLights[i], position,diffuse,spec,normal,viewDir,shininess);

				totalAmbient += res.Ambient.xyz;
				totalDiffuse = res.Diffuse.xyz;
				totalSpecular = res.Specular.xyz;
			
			}

		}
		float shadow = CalcShadowValue(position,normal);

		vec3 res = totalAmbient + (shadow)*(totalDiffuse + totalSpecular);

		vec4 resultColor = vec4(materialColor.xyz * (res),1.0);
		
		fragCol += resultColor;
	}

	fragCol/= float(u_MSAASampleCount);
	
	o_FragColor = fragCol;
}
