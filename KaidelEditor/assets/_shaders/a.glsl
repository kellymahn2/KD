#version 460 core
#include "SceneData.glsli"
#include "Sampler.glsli"
#ifdef vertex
layout(location = 0)in vec3 a_Position;
layout(location = 1)in vec2 a_TexCoords;
layout(location = 2)in vec3 a_Normal;
layout(location = 3)in vec3 a_Tangent;
layout(location = 4)in vec3 a_BiTangent;

layout(push_constant) uniform DrawData {
        uint InstanceOffset;
};

layout(set = 2, binding = 0) buffer _InstanceData
{
        mat4 InstanceTransform[];
};

layout(set = 1, binding = 0) uniform _SceneData
{
        SceneData u_SceneData;
};

layout(location = 0) out VS_OUT {
        vec3 FragPos;
        vec3 T;
        vec3 N;
        vec3 B;
        vec2 TexCoords;
} Output;

void main() {
        mat4 transform = InstanceTransform[gl_InstanceIndex + InstanceOffset];

        vec4 pos = transform * vec4(a_Position, 1.0);

        gl_Position = u_SceneData.ViewProj * pos;

        mat3 normalMat = mat3(transform);

        Output.FragPos = vec3(pos);
        Output.T = normalize(normalMat * a_Tangent);
        Output.B = normalize(normalMat * a_BiTangent);
        Output.N = normalize(normalMat * a_Normal);
        Output.T = Output.T - (dot(Output.T, Output.N)) * Output.N;
        Output.B = Output.B - (dot(Output.B, Output.N)) * Output.N - (dot(Output.B, Output.T)) * Output.T;
        Output.TexCoords = a_TexCoords;

}
#endif

#ifdef fragment
layout (location = 0) out vec4 o_Position;
layout (location = 1) out vec4 o_Normal;
layout (location = 2) out vec4 o_Albedo;
layout (location = 3) out vec2 o_MetallicRoughness;
layout (location = 4) out vec4 o_Emissive;
layout(location = 0) in vec3 FragPos;
void main() {
        o_Position = vec4(FragPos, 1.0);
        vec3 Normal = vec3(0.0, 0.0, 1.0);
		//vec4 Albedo(1.0), Emissive(0.0);
        //float Metallic = 0.0, Roughness = 1.0;
        //o_Albedo = Albedo; o_Normal = vec4(Normal, 1.0); o_Emissive = Emissive; o_MetallicRoughness = vec2(Metallic, Roughness);
}
#endif
