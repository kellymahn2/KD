#version 460 core

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

struct VertexOutput
{
	vec4 Color;
};

layout (location = 0) in VertexOutput Input;
layout (location = 1) in flat int v_EntityID;


void main()
{
	
	color = Input.Color;
	color2 = v_EntityID;
}
