#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoords;

layout(push_constant) uniform PushConstants{
	mat4 ViewProjection;
};

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TexCoords;

void main(){
	v_Color = a_Color;
	v_TexCoords = a_TexCoords.xy;

	gl_Position = ViewProjection * vec4(a_Position, 1.0);
	gl_Position.y *= -1.0;
}

#type fragment
#version 460 core

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoords;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 0) uniform texture2D u_FontAtlas;
layout(set = 0, binding = 1) uniform sampler u_Sampler;

const float pxRange = 2.0;

float screenPxRange() {
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(sampler2D(u_FontAtlas, u_Sampler), 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(v_TexCoords);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
	vec3 msd = texture(sampler2D(u_FontAtlas, u_Sampler), v_TexCoords).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange()*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

	if(opacity == 0.0)
		discard;

	vec4 bgColor = vec4(0.0);
    o_Color = mix(bgColor, v_Color, opacity);
	if(o_Color.a == 0.0)
		discard;
}


