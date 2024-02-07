#version 460 core


layout(location = 0)out vec4 o_Color;


layout(location = 0)in vec2 v_TexCoords;
layout(location = 1)in flat int v_MaterialID;


struct Material2D {
	float ColorX,ColorY,ColorZ,ColorW;
	int Albedo;
};

layout(binding = 0) uniform sampler2DArray u_Material2DTextures;
layout(std430,binding = 1) buffer Material2Ds{
	Material2D u_Material2Ds[];
};


void main(){
	vec4 col = vec4(u_Material2Ds[v_MaterialID].ColorX,u_Material2Ds[v_MaterialID].ColorY,u_Material2Ds[v_MaterialID].ColorZ,u_Material2Ds[v_MaterialID].ColorW);
	o_Color = texture(u_Material2DTextures,vec3(v_TexCoords,u_Material2Ds[v_MaterialID].Albedo)) * col;
}
