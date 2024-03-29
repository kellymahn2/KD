#version 460 core
layout(location = 0) in vec2 a_Position;


layout(location = 0)uniform vec2 u_ScreenSize;



#include "FullScreenQuadOutput.glsl"

layout(location = 0) out FragmentData v_FragmentData;


void main(){
	v_FragmentData.TexCoords = a_Position *.5 + .5;
	v_FragmentData.ScreenPosition = (a_Position + 1.0) * .5 * u_ScreenSize;
	gl_Position = vec4(a_Position,0.0,1.0);
}
