#type vertex
#version 460 core
layout(location = 0) in vec2 a_NDC;
layout(location = 1) in vec2 a_TexCoords;

layout(location = 0)out vec2 v_TexCoords;

void main(){
	gl_Position = vec4(a_NDC,0.0,1.0);

	v_TexCoords = a_TexCoords;
}
#type fragment
#version 460 core

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoords;

layout(set = 0,binding = 0) uniform sampler GlobalSampler;
layout(set = 0,binding = 1) uniform texture2D HDRTexture;

void main(){
    vec3 hdrCol = texture(sampler2D(HDRTexture,GlobalSampler), v_TexCoords).rgb;

    //Exposure tone mapping
    vec3 toneMappedResult = vec3(1.0) - exp(-hdrCol);

    //reinhard tone mapping
    // vec3 toneMappedResult = hdrCol / (hdrCol + vec3(1.0));

    o_Color = vec4(toneMappedResult, 1.0) ;
}
