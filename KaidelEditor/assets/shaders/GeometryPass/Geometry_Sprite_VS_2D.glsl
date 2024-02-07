#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;
layout(location = 2) in int a_MaterialID;


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec3 u_CameraPosition;
};

layout(location = 0)out vec2 v_TexCoords;
layout(location = 1)out flat int v_MaterialID;

void main(){
	v_MaterialID = a_MaterialID;
	v_TexCoords = a_TexCoords;
	gl_Position = u_ViewProjection * vec4(a_Position,1.0);
}
