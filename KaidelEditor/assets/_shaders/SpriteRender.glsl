#version 460 core
#ifdef vertex

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec3 a_TexCoords;


layout(push_constant) uniform PushConstants{
	mat4 ViewProjection;
};


layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TexCoords;
layout(location = 2) out flat int v_TextureIndex;


void main(){
	v_Color = a_Color;
	v_TexCoords = a_TexCoords.xy;
	v_TextureIndex = int(a_TexCoords.z);

	gl_Position = ViewProjection * vec4(a_Position, 1.0);
}
#endif


#ifdef fragment

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoords;
layout(location = 2) in flat int v_TextureIndex;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 0) uniform texture2D u_Textures[32];
layout(set = 0, binding = 1) uniform sampler u_Sampler;

void main(){
	vec4 texColor = texture(sampler2D(u_Textures[v_TextureIndex], u_Sampler), v_TexCoords);

	o_Color = texColor;
	o_Color.a = 1.0;
}
#endif
