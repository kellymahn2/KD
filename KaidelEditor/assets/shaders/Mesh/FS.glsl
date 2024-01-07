#version 460 core

struct VSOutput{
    vec2 TexCoords;
};

layout(location = 0)in VSOutput Input;
layout(location = 1)in flat int v_MaterialIndex;

layout(location = 0)out vec4 o_Color;


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec3 u_CameraPosition;
};


struct Material{
	float ColorX,ColorY,ColorZ,ColorW;
	int Diffuse;
	int Specular;
	float Shininess;
};

layout(std430,binding = 1) buffer Materials{
	Material u_Materials[];
};
layout(binding = 0) uniform sampler2DArray u_MaterialTextures;


void main(){
    vec4 diffuse = vec4(1.0);
    o_Color = diffuse;
}