#version 460 core
layout (location = 0) out vec3 o_Position;
layout (location = 1) out vec3 o_Normal;
layout(location = 2) out int o_MaterialIndex;
layout (location = 3) out vec4 o_DiffuseSpec;
struct VSOutput{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
};

layout(location = 0)in VSOutput Input;
layout(location = 3)in flat int u_MatIndex;

struct Material{
	float ColorX,ColorY,ColorZ,ColorW;
	int Diffuse;
	int Specular;
	float Shininess;
};



	layout(binding = 0) uniform sampler2DArray u_MaterialTextures;
layout(std430,binding = 1) buffer Materials{
	Material u_Materials[];
};


void main(){
	o_Position =  Input.Position;
	o_Normal = Input.Normal;
	o_MaterialIndex = u_MatIndex;
	o_DiffuseSpec.rgb = texture(u_MaterialTextures,vec3(Input.TexCoords,u_Materials[o_MaterialIndex].Diffuse)).rgb;
	o_DiffuseSpec.a = texture(u_MaterialTextures,vec3(Input.TexCoords,u_Materials[o_MaterialIndex].Specular)).r;
}
