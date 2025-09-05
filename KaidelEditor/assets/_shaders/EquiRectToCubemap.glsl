#version 460 core
#ifdef compute
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

const float PI = 3.1415926535897932384626433832795;
const float TwoPI = 2 * PI;


layout(set = 0, binding = 0) uniform sampler2D u_EquiRectTexture;
layout(set = 0, binding = 1, rgba16f) restrict writeonly uniform imageCube u_Cube;

vec3 getSamplingVector()
{
    vec2 st = gl_GlobalInvocationID.xy/vec2(imageSize(u_Cube));
    vec2 uv = 2.0 * vec2(st.x, 1.0-st.y) - vec2(1.0);

    vec3 ret;
    if(gl_GlobalInvocationID.z == 0)      ret = vec3(1.0,  uv.y, -uv.x);
    else if(gl_GlobalInvocationID.z == 1) ret = vec3(-1.0, uv.y,  uv.x);
    else if(gl_GlobalInvocationID.z == 2) ret = vec3(uv.x, 1.0, -uv.y);
    else if(gl_GlobalInvocationID.z == 3) ret = vec3(uv.x, -1.0, uv.y);
    else if(gl_GlobalInvocationID.z == 4) ret = vec3(uv.x, uv.y, 1.0);
    else if(gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y, -1.0);
    return normalize(ret);
}

void main(void)
{
	vec3 v = getSamplingVector();

	// Convert Cartesian direction vector to spherical coordinates.
	float phi   = atan(v.z, v.x);
	float theta = acos(v.y);

	// Sample equirectangular texture.
	vec4 color = texture(u_EquiRectTexture, vec2(phi/TwoPI, theta/PI));

	// Write out color to output cubemap.
	imageStore(u_Cube, ivec3(gl_GlobalInvocationID), color);
}

#endif
