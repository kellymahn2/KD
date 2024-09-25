#type vertex
#version 460 core

layout(location = 0)in vec3 a_Position;
layout(location = 1)in vec2 a_TexCoords;
layout(location = 2)in vec3 a_Normal;
layout(location = 3)in vec3 a_Tangent;
layout(location = 4)in vec3 a_BiTangent;

layout(push_constant) uniform DrawData{
	mat4 ViewProj;
};

layout(location = 0) out VS_OUT{
	vec3 FragPos;
	vec3 T;
	vec3 N;
	vec3 B;
	vec2 TexCoords;
} Output;

layout(set = 0,binding = 0)buffer instance{
	mat4 Transforms[];
};

void main(){
	mat4 transform = Transforms[gl_InstanceIndex];
	vec4 pos = transform * vec4(a_Position,1.0);
	
	gl_Position = ViewProj * pos;

	Output.FragPos = vec3(pos);
	Output.T = normalize(mat3(transform) * a_Tangent);
	Output.B = normalize(mat3(transform) * a_BiTangent);
	Output.N = normalize(mat3(transform) * a_Normal);
	Output.TexCoords = a_TexCoords;
}

#type fragment
#version 460 core

layout (location = 0) out vec4 o_Position;
layout (location = 1) out vec4 o_Normal;
layout (location = 2) out vec4 o_Albedo;
layout (location = 3) out vec2 o_MetallicRoughness;

layout(push_constant) uniform DrawData{
	mat4 ViewProj;
};

layout(location = 0) in VS_OUT{
	vec3 FragPos;
	vec3 T;
	vec3 N;
	vec3 B;
	vec2 TexCoords;
} Input;

layout(set = 1,binding = 0) uniform sampler GlobalSampler;
layout(set = 1,binding = 1) uniform texture2D Albedo;
layout(set = 1,binding = 2) uniform texture2D NormalMap;
layout(set = 1,binding = 3) uniform texture2D MetallicMap;
layout(set = 1,binding = 4) uniform texture2D RoughnessMap;

void main(){
	o_Position = vec4(Input.FragPos,1.0);

	vec3 normal = normalize(2.0 * texture(sampler2D(NormalMap,GlobalSampler),Input.TexCoords).rgb - 1.0);
	mat3 TBN  = mat3(Input.T, Input.B, Input.N);
	vec3 norm = normalize(TBN * normal);
	o_Normal = vec4(norm,1.0);
	o_Albedo = vec4(texture(sampler2D(Albedo,GlobalSampler),Input.TexCoords).rgba);
	o_MetallicRoughness = vec2(
				texture(sampler2D(MetallicMap,GlobalSampler),Input.TexCoords).b,
				texture(sampler2D(RoughnessMap,GlobalSampler),Input.TexCoords).g);
}
