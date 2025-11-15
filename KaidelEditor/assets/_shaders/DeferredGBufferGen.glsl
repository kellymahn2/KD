#version 460 core
#include "SceneData.glsli"
#include "Sampler.glsli"

#ifdef vertex

layout(location = 0)in vec3 a_Position;
layout(location = 1)in vec2 a_TexCoords;
layout(location = 2)in vec3 a_Normal;
layout(location = 3)in vec3 a_Tangent;
layout(location = 4)in vec3 a_BiTangent;

layout(push_constant) uniform DrawData{
	uint InstanceOffset;
};

layout(set = 2, binding = 0) buffer _InstanceData
{
	mat4 InstanceTransform[];
};

layout(set = 1, binding = 0) uniform _SceneData
{
	SceneData u_SceneData;
};

layout(location = 0) out VS_OUT{
	vec3 FragPos;
	vec3 T;
	vec3 N;
	vec3 B;
	vec2 TexCoords;
} Output;

void main() {
	mat4 transform = InstanceTransform[gl_InstanceIndex + InstanceOffset];

	vec4 pos = transform * vec4(a_Position,1.0);
	
	gl_Position = u_SceneData.ViewProj * pos;

	mat3 normalMat = mat3(transform);

	Output.FragPos = vec3(pos);
	Output.T = normalize(normalMat * a_Tangent);
	Output.B = normalize(normalMat * a_BiTangent);
	Output.N = normalize(normalMat * a_Normal);
	Output.T = Output.T - (dot(Output.T,Output.N)) * Output.N;
	Output.B = Output.B - (dot(Output.B,Output.N)) * Output.N - (dot(Output.B,Output.T)) * Output.T;
	Output.TexCoords = a_TexCoords;

}
#endif

#ifdef fragment
layout (location = 0) out vec4 o_Position;
layout (location = 1) out vec4 o_Normal;
layout (location = 2) out vec4 o_Albedo;
layout (location = 3) out vec2 o_MetallicRoughness;
layout (location = 4) out vec4 o_Emissive;

layout(location = 0) in VS_OUT{
	vec3 FragPos;
	vec3 T;
	vec3 N;
	vec3 B;
	vec2 TexCoords;
} Input;

layout(set = 3,binding = 0) uniform texture2D Albedo;
layout(set = 3,binding = 1) uniform texture2D NormalMap;
layout(set = 3,binding = 2) uniform texture2D MetallicMap;
layout(set = 3,binding = 3) uniform texture2D RoughnessMap;
layout(set = 3, binding = 4) uniform texture2D EmissionMap;

layout(set = 4, binding = 0, std140) uniform MaterialUniformData
{
	vec4 BaseColor;
	float Roughness;
	float Metalness;

	uint MetalnessChannel;
	uint RoughnessChannel;

	uint HasAlbedo;
	uint HasNormal;
	uint HasMetallic;
	uint HasRoughness;
	uint HasEmission;
	
} MaterialData;

void main(){
	o_Position = vec4(Input.FragPos,1.0);
	
	vec3 normal = vec3(0.0, 0.0, 1.0);
	if(MaterialData.HasNormal == 1)
		normal = normalize(2.0 * texture(sampler2D(NormalMap,SAMPLER_LINEAR_MIPPED_CLAMP),Input.TexCoords).rgb - 1.0);
	
	mat3 TBN  = mat3(Input.T, Input.B, Input.N);
	vec3 norm = normalize(TBN * normal);
	o_Normal = vec4(norm, 1.0);

	vec4 baseColor = MaterialData.BaseColor;
	if(MaterialData.HasAlbedo == 1)
		baseColor *= pow(vec4(texture(sampler2D(Albedo,SAMPLER_LINEAR_MIPPED_CLAMP),Input.TexCoords).rgba), vec4(2.2));

	o_Albedo = baseColor;
	
	float metallic = MaterialData.Metalness;
	float roughness = MaterialData.Roughness;

	if(MaterialData.HasMetallic == 1)
	{
		vec4 metallicColor = texture(sampler2D(MetallicMap,SAMPLER_LINEAR_MIPPED_CLAMP),Input.TexCoords);
		if(MaterialData.MetalnessChannel == 0)
		{
			metallic = metallicColor.r;
		}
		else if(MaterialData.MetalnessChannel == 1)
		{
			metallic = metallicColor.g;
		}
		else if(MaterialData.MetalnessChannel == 2)
		{
			metallic = metallicColor.b;
		}
	}

	if(MaterialData.HasRoughness == 1)
	{
		vec4 roughnessColor = texture(sampler2D(RoughnessMap,SAMPLER_LINEAR_MIPPED_CLAMP),Input.TexCoords);

		if(MaterialData.RoughnessChannel == 0)
		{
			roughness = roughnessColor.r;
		}
		else if(MaterialData.RoughnessChannel == 1)
		{
			roughness = roughnessColor.g;
		}
		else if(MaterialData.RoughnessChannel == 2)
		{
			roughness = roughnessColor.b;
		}
	}

	o_MetallicRoughness = vec2(metallic, roughness);

	// if(MaterialData.HasEmission == 1)
	// {
	// 	o_Emissive = pow(vec4(texture(sampler2D(EmissionMap,SAMPLER_LINEAR_MIPPED_CLAMP),Input.TexCoords).rgba), vec4(2.2));
	// }
	// else 
	{
		o_Emissive = vec4(0.0);
	}
}
#endif
