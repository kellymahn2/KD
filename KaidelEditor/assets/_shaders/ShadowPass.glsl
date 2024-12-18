#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;

layout(push_constant) uniform PushConstants {
    mat4 Model;           // Model matrix
};

void main() {
    // Transform the vertex position to world space
    gl_Position = Model * vec4(a_Position, 1.0);
}

#type geometry
#version 460 core
layout(triangles, invocations = 4) in;                         // Input: triangles
layout(triangle_strip, max_vertices = 3) out; // Output: triangle strip

layout(push_constant) uniform PushConstants {
    mat4 Model;           // Model matrix
};

layout(set = 0,binding = 0, std140) uniform DirLight{
	mat4 GlobalShadowMatrix;
	mat4 ViewProjection[4];
	vec3 Direction;
	vec3 Color;
	vec4 SplitDistances;
	vec4 CascadeOffsets[4];
	vec4 CascadeScales[4];
	float FadeStart;
} DLight;

// Main function for the geometry shader  
void main() {  
	for(int i = 0;i<gl_in.length();++i){
		gl_Layer = int(gl_InvocationID);
		gl_Position = DLight.ViewProjection[gl_InvocationID] * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}  
