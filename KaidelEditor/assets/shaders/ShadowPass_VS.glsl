#version 460 core
layout(location = 0)in vec3 a_Position;
layout(location = 1)in mat4 a_Transform;

uniform int u_LightIndex;

#include "Core/Core.glsl"

void main(){
	gl_Position = u_SpotLights[u_LightIndex].LightViewProjection * a_Transform * vec4(a_Position,1.0);
}
