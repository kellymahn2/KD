#version 460 core
#include "PCFSamplingDisk.glsli" 
#include "Sampler.glsli"
#include "PBRFunctions.glsli"
#include "SceneData.glsli"
#ifdef vertex
const vec2 positions[3] = vec2[3](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));

layout(location = 0)out vec2 v_TexCoords;

void main(){
	gl_Position = vec4(positions[gl_VertexIndex],0.0,1.0);
	v_TexCoords = 0.5 * gl_Position.xy + vec2(0.5);
   }
#endif

#ifdef fragment


const int SplitCount = 4;
const int PCFSamples = 9;
const int ShadowMapSize = 2048;

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoords;

layout(set = 1,binding = 0) uniform texture2D Positions;
layout(set = 1,binding = 1) uniform texture2D Normals;
layout(set = 1,binding = 2) uniform texture2D Albedo;
layout(set = 1,binding = 3) uniform texture2D MetallicRoughness;
layout(set = 1,binding = 4) uniform texture2D EmissionMap;

layout(set = 3,binding = 0, std140) uniform DirLight{
	mat4 ViewProjection[4];
	vec3 Direction;
	vec3 Color;
	vec4 SplitDistances; 
	float FadeStart;
	float LightSize;
} DLight;
layout(set = 3,binding = 1) uniform texture2D DirectionalShadows[4];

layout(set = 2, binding = 0) uniform samplerCube EnvironmentMap;
layout(set = 2, binding = 1) uniform samplerCube IrradianceMap;
layout(set = 2, binding = 2) uniform sampler2D SpecularLUT;
layout(set = 2, binding = 3) uniform samplerCube Specular;

layout(set = 4, binding = 0, std140) uniform _SceneData {
	SceneData u_SceneData;
};

vec4 SRGBtoLINEAR(vec4 srgbIn)
{
	vec3 linOut = pow(srgbIn.xyz,vec3(2.2));
	return vec4(linOut,1.0);
}

vec3 Uncharted2Tonemap(vec3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec4 tonemap(vec4 color)
{
	vec3 outcol = Uncharted2Tonemap(color.rgb * 4.5);
	outcol = outcol * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	return vec4(pow(outcol, vec3(1.0f / 2.2)), color.a);
}


vec3 CalcAmbientLighting(vec3 N, vec3 albedo, vec3 F0, float metalness, float roughness, vec3 Lo, float cosLo)
{
	vec3 Lr = reflect(-Lo, N);
	vec3 F = fresnelSchlickRoughness(cosLo, F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metalness;	  
    
    vec3 irradiance = SRGBtoLINEAR(tonemap(texture(IrradianceMap, N))).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    const float MAX_REFLECTION_LOD = textureQueryLevels(Specular);
    vec3 prefilteredColor = SRGBtoLINEAR(tonemap(textureLod(Specular, Lr,  roughness * (MAX_REFLECTION_LOD)))).rgb;    
    vec2 brdf  = texture(SpecularLUT, vec2(cosLo, roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

	vec3 ambientLighting = (kD * diffuse + specular);
	return ambientLighting;
}

uint CalcCascadeIndex(float viewDepth, vec4 splits){
	uint cascadeIndex = 3;

	for(int i = 3; i >= 0; --i) {
		if(viewDepth <= splits[i]) 
			cascadeIndex = i;
	}
	return cascadeIndex;
}

float sampleShadowPCFBasic(vec2 uv, float cmp, in texture2D cascade)
{
	float shadow = 0.0;

	shadow += texture      (sampler2D(cascade, SAMPLER_NEAREST_CLAMP), uv).r < cmp ? 0.0 : 1.0;
	shadow += textureOffset(sampler2D(cascade, SAMPLER_NEAREST_CLAMP), uv, ivec2(-1, -1)).r < cmp ? 0.0 : 1.0;
	shadow += textureOffset(sampler2D(cascade, SAMPLER_NEAREST_CLAMP), uv, ivec2(-1, 0)).r < cmp ? 0.0 : 1.0;
	shadow += textureOffset(sampler2D(cascade, SAMPLER_NEAREST_CLAMP), uv, ivec2(-1, 1)).r < cmp ? 0.0 : 1.0;
	shadow += textureOffset(sampler2D(cascade, SAMPLER_NEAREST_CLAMP), uv, ivec2(0, -1)).r < cmp ? 0.0 : 1.0;
	shadow += textureOffset(sampler2D(cascade, SAMPLER_NEAREST_CLAMP), uv, ivec2(0, 1)).r < cmp ? 0.0 : 1.0;
	shadow += textureOffset(sampler2D(cascade, SAMPLER_NEAREST_CLAMP), uv, ivec2(1, -1)).r < cmp ? 0.0 : 1.0;
	shadow += textureOffset(sampler2D(cascade, SAMPLER_NEAREST_CLAMP), uv, ivec2(1, 0)).r < cmp ? 0.0 : 1.0;
	shadow += textureOffset(sampler2D(cascade, SAMPLER_NEAREST_CLAMP), uv, ivec2(1, 1)).r < cmp ? 0.0 : 1.0;

	shadow /= 9.0;

	return shadow;
}

float sampleShadowPCFDisk(vec2 uv, float cmp, float lightRadius, in texture2D cascade)
{
	float shadow = 0;
	
	vec2 shadowAtlasRes = vec2(textureSize(sampler2D(cascade, SAMPLER_NEAREST_CLAMP), 0).xy);
	vec2 shadowAtlasRcp = vec2(1.0) / shadowAtlasRes;

	vec2 spread = shadowAtlasRcp * (lightRadius * 8 + 2); // remap radius to try to match ray traced shadow result

	for (uint i = 0; i < soft_shadow_sample_count; ++i)
	{
		vec2 sample_uv = vogel_points[i] * spread + uv;

		sample_uv = clamp(sample_uv, 0.0, 1.0);
		float pcf = texture(sampler2D(cascade, SAMPLER_NEAREST_CLAMP), sample_uv).r < cmp ? 0.0 : 1.0;
		

		shadow += pcf;
		
	}
	shadow *= soft_shadow_sample_count_rcp;

	return shadow;
}

float calcDirShadow(vec3 worldPos,vec3 norm,vec3 lightDir)  
{
	vec4 colors[4] = vec4[4](vec4(1.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0), vec4(0.0, 0.0, 1.0, 1.0), vec4(1.0, 1.0, 0.0, 1.0));

	float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);  

	for(uint cascade = 0; cascade < 4; ++cascade)
	{
		// Project into shadow map space (no need to divide by .w because ortho projection!):
		const mat4 cascade_projection = DLight.ViewProjection[cascade];
		vec3 shadow_pos = (cascade_projection * vec4(worldPos, 1.0)).xyz;
		vec3 shadow_uv = shadow_pos * vec3(0.5, 0.5, 1.0) + vec3(0.5, 0.5, 0.0);
		// Determine if pixel is inside current cascade bounds and compute shadow if it is:
		if(shadow_uv == clamp(shadow_uv, 0.0, 1.0))	
		{
			const float shadow_main = sampleShadowPCFDisk(shadow_uv.xy, shadow_uv.z - bias, DLight.LightSize, DirectionalShadows[cascade]);
			//const float shadow_main = sampleShadowPCFBasic(shadow_uv.xy, shadow_uv.z - bias, DirectionalShadows[cascade]);
			return shadow_main;
		}

	}
} 

void main()
{
	float ndcDepth = gl_FragCoord.z;
	
	if(ndcDepth == 1.0){
		discard;
	}

	ivec2 texCoords = ivec2(vec2(v_TexCoords.x, v_TexCoords.y) * u_SceneData.ScreenSize);

	vec3 position = texelFetch(sampler2D(Positions, SAMPLER_NEAREST_CLAMP), texCoords, 0).rgb;
	vec3 normal = normalize(texelFetch(sampler2D(Normals, SAMPLER_NEAREST_CLAMP), texCoords, 0).rgb);
	vec3 albedo = texelFetch(sampler2D(Albedo, SAMPLER_NEAREST_CLAMP), texCoords, 0).rgb;
	vec2 mr = texelFetch(sampler2D(MetallicRoughness, SAMPLER_NEAREST_CLAMP), texCoords, 0).rg;
	vec3 emission = texelFetch(sampler2D(EmissionMap, SAMPLER_NEAREST_CLAMP), texCoords, 0).rgb;

	float metalness = mr.r;
	float roughness = mr.g;

	vec3 N = normal;

	vec3 Lo = normalize(u_SceneData.CameraPos - position);
	
	float cosLo = max(0.0, dot(N, Lo));
	
	vec3 F0 = mix(vec3(0.04), albedo, metalness);

	
	vec3 directionalLight = vec3(0.0);
	
	{
		vec3 Li = -DLight.Direction;
		vec3 Lradiance = DLight.Color;
		
		// Half-vector between Li and Lo.
		vec3 Lh = normalize(Li + Lo);
		
		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(N, Li));
		float cosLh = max(0.0, dot(N, Lh));
		
		// Calculate Fresnel term for direct lighting. 
		vec3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
		// Calculate normal distribution for specular BRDF.
		float D = ndf(cosLh, roughness);
		// Calculate geometric attenuation for specular BRDF.
		float G = gaSchlickGGX(cosLi, cosLo, roughness);
		
		// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
		// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
		// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);
		
		// Lambert diffuse BRDF.
		// We don't scale by 1/PI for lighting & material units to be more convenient.
		// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
		vec3 diffuseBRDF = kd * albedo;
		
		// Cook-Torrance specular microfacet BRDF.
		vec3 specularBRDF = (F * D * G) / max(0.0001, 4.0 * cosLi * cosLo);


		float shadowWeight = calcDirShadow(position, N, -normalize(DLight.Direction));
		// Total contribution for this light.
		directionalLight += shadowWeight * (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
	}
	
	vec3 ambientLighting = CalcAmbientLighting(N, albedo, F0, metalness, roughness, Lo, cosLo);

	vec3 color = ambientLighting + directionalLight + emission;

	o_Color = vec4(color, 1.0);
}
#endif
