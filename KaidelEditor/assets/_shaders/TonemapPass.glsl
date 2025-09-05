#version 460 core
#include "Sampler.glsli"
#ifdef vertex
const vec2 positions[3] = vec2[3](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));

layout(location = 0)out vec2 v_TexCoords;

void main(){
	gl_Position = vec4(positions[gl_VertexIndex],0.0,1.0);
	v_TexCoords = 0.5 * gl_Position.xy + vec2(0.5);
	gl_Position.y *= -1.0;
}
#endif

#ifdef fragment

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoords;


layout(set = 1,binding = 0) uniform texture2D HDRTexture;


const float gamma     = 2.2;
const float exposure  = 1.0;
const float pureWhite = 1.0;

void main(){
	ivec2 texCoords = ivec2(vec2(v_TexCoords.x, v_TexCoords.y) * textureSize(sampler2D(HDRTexture, SAMPLER_NEAREST_CLAMP), 0).xy);

    vec3 hdrCol = texelFetch(sampler2D(HDRTexture, SAMPLER_NEAREST_CLAMP), texCoords, 0).rgb;

    // // Reinhard tonemapping operator.
	// // see: "Photographic Tone Reproduction for Digital Images", eq. 4
	// float luminance = dot(hdrCol, vec3(0.2126, 0.7152, 0.0722));
	// float mappedLuminance = (luminance * (1.0 + luminance/(pureWhite*pureWhite))) / (1.0 + luminance);

	// // Scale color by ratio of average luminances.
	// vec3 mappedColor = (mappedLuminance / luminance) * hdrCol;

	// // Gamma correction.
	// o_Color = vec4(pow(mappedColor, vec3(1.0/gamma)), 1.0);

	vec3 mappedColor = hdrCol / (1 + hdrCol);

	mappedColor = pow(mappedColor, vec3(1.0 / 2.2));

	o_Color = vec4(mappedColor, 1.0);
}

#endif