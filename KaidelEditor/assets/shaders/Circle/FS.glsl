#version 460 core

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

struct VertexOutput
{
	vec3 LocalPosition;
	vec4 Color;
	float Thickness;
	float Fade;

};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat int v_EntityID;


void main()
{

	float distance = 1.0-length(Input.LocalPosition);

	float c= smoothstep(0.0,Input.Fade,distance);
	c*=smoothstep(Input.Thickness+Input.Fade,Input.Thickness,distance);

	if(c==0.0)
		discard;

	color =Input.Color;
	color.a *= c;
		
	color2 = v_EntityID;
}
