#version 460 core
#ifdef vertex


const vec3 Cube[36] = vec3[36](
	// Front face
	vec3(-1.0, -1.0,  1.0),
	vec3(1.0, -1.0,  1.0),
	vec3(1.0,  1.0,  1.0),
	vec3(1.0,  1.0,  1.0),
	vec3(-1.0,  1.0,  1.0),
	vec3(-1.0, -1.0,  1.0),

	// Back face
	vec3(-1.0, -1.0, -1.0),
	vec3(1.0, -1.0, -1.0),
	vec3(1.0,  1.0, -1.0),
	vec3(1.0,  1.0, -1.0),
	vec3(-1.0,  1.0, -1.0),
	vec3(-1.0, -1.0, -1.0),

	// Left face
	vec3(-1.0, -1.0, -1.0),
	vec3(-1.0, -1.0,  1.0),
	vec3(-1.0,  1.0,  1.0),
	vec3(-1.0,  1.0,  1.0),
	vec3(-1.0,  1.0, -1.0),
	vec3(-1.0, -1.0, -1.0),

	// Right face
	vec3(1.0, -1.0, -1.0),
	vec3(1.0, -1.0,  1.0),
	vec3(1.0,  1.0,  1.0),
	vec3(1.0,  1.0,  1.0),
	vec3(1.0,  1.0, -1.0),
	vec3(1.0, -1.0, -1.0),

	// Top face
	vec3(-1.0,  1.0, -1.0),
	vec3(-1.0,  1.0,  1.0),
	vec3(1.0,  1.0,  1.0),
	vec3(1.0,  1.0,  1.0),
	vec3(1.0,  1.0, -1.0),
	vec3(-1.0,  1.0, -1.0),

	// Bottom face
	vec3(-1.0, -1.0, -1.0),
	vec3(1.0, -1.0, -1.0),
	vec3(1.0, -1.0,  1.0),
	vec3(1.0, -1.0,  1.0),
	vec3(-1.0, -1.0,  1.0),
	vec3(-1.0, -1.0, -1.0)
);


layout(location = 0) in vec3 a_Position;

layout(location = 0) out vec3 v_TexCoords;

layout(push_constant) uniform PushConstants
{
    mat4 Projection;
    mat4 View;
};

void main()
{
    vec3 pos = a_Position;
    v_TexCoords = pos;
    gl_Position = Projection * mat4(mat3(View)) * vec4(pos, 1.0);
    gl_Position = gl_Position.xyww;

}
#endif

#ifdef fragment

layout(location = 0) in vec3 v_TexCoords;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 0) uniform samplerCube EnvironmentMap;
layout(set = 0, binding = 1) uniform samplerCube IrradianceMap;
layout(set = 0, binding = 2) uniform sampler2D SpecularLUT;
layout(set = 0, binding = 3) uniform samplerCube Specular;

void main()
{    
    o_Color = texture(EnvironmentMap, v_TexCoords);
}
#endif
