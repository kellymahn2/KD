#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

layout(location = 0) out vec4 v_FragColor;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec3 u_CameraPosition;
};

void main() {
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
	v_FragColor = a_Color;
}
