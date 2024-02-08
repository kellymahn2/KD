#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec4 a_PatchDef;

struct LineVertex {
	vec3 Position;
	vec4 Color;
	//X:Increment, Y:Start, Z:End, W:Index
	vec4 PatchDef;
};

layout(location = 0) out v_Vertices[];

void main(){
	LineVertex vertex;
	vertex.Position = a_Position;
	vertex.Color = a_Color; 
	vertex.PatchDef = a_PatchDef;
	
	v_Vertices[int(a_PatchDef.w)] = vertex;

	gl_Position = vec4(a_Position,1.0);
}

