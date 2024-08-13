#version 450 core

layout(location = 0) in vec4 v_FragColor;
layout(location = 1) in vec3 v_UVCoords;

layout(location = 0) out vec4 o_OutColor;

layout(set = 1,binding = 0) uniform sampler2DArray u_LoadedTextures;


void main() {
	vec4 col = texture(u_LoadedTextures,v_UVCoords);
	o_OutColor = vec4(v_UVCoords.x,v_UVCoords.y,0.0,1.0);
	o_OutColor = col;
	o_OutColor.w = 1.0;
}
