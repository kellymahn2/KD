#version 460 core
#ifdef compute
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(set = 0, binding = 0, std430) buffer restrict readonly SrcVertexData{
	uint Data[];
} SrcVertices;


layout(set = 0, binding = 1, std430) buffer restrict writeonly DstVertexData{
	uint Data[];
} DstVertices;

layout(set = 1, binding = 0, std430) buffer restrict readonly SkeletonData {
	mat4 Data[];
} BoneTransforms;
layout(set = 1, binding = 1, std430) buffer restrict readonly OffsetsData {
	mat4 Data[];
} BoneOffsets;

struct MeshVertex
{
	vec3 Position;
	vec2 TexCoords;
	vec3 ModelNormal;
	vec3 ModelTangent;
	vec3 ModelBitangent;
};

struct SkinnedMeshVertex 
{
	vec3 Position;
	vec2 TexCoords;
	vec3 ModelNormal;
	vec3 ModelTangent;
	vec3 ModelBitangent;
	int BoneID[4];
	float BoneWeight[4];
};

float decodeFloat(uint v0)
{
	float value = uintBitsToFloat(v0);

	return value;
}

vec2 decodeVec2(uint v0, uint v1)
{
	vec2 value = vec2(decodeFloat(v0), decodeFloat(v1));

	return value;
}

vec3 decodeVec3(uint v0, uint v1, uint v2)
{
	vec3 value = vec3(decodeFloat(v0), decodeFloat(v1), decodeFloat(v2));

	return value;
}

SkinnedMeshVertex decodeSource(uint index) 
{
	SkinnedMeshVertex vertex;
	vertex.Position			= decodeVec3(SrcVertices.Data[index + 0U], SrcVertices.Data[index + 1U], SrcVertices.Data[index + 2U]); index += 3U;
	vertex.TexCoords		= decodeVec2(SrcVertices.Data[index + 0U], SrcVertices.Data[index + 1U]);								index += 2U;
	vertex.ModelNormal		= decodeVec3(SrcVertices.Data[index + 0U], SrcVertices.Data[index + 1U], SrcVertices.Data[index + 2U]); index += 3U;
	vertex.ModelTangent		= decodeVec3(SrcVertices.Data[index + 0U], SrcVertices.Data[index + 1U], SrcVertices.Data[index + 2U]); index += 3U;
	vertex.ModelBitangent	= decodeVec3(SrcVertices.Data[index + 0U], SrcVertices.Data[index + 1U], SrcVertices.Data[index + 2U]); index += 3U;
	
	vertex.BoneID[0] = int(SrcVertices.Data[index + 0U]); index += 1U;
	vertex.BoneID[1] = int(SrcVertices.Data[index + 0U]); index += 1U;
	vertex.BoneID[2] = int(SrcVertices.Data[index + 0U]); index += 1U;
	vertex.BoneID[3] = int(SrcVertices.Data[index + 0U]); index += 1U;

	vertex.BoneWeight[0] = decodeFloat(SrcVertices.Data[index + 0U]); index += 1U;
	vertex.BoneWeight[1] = decodeFloat(SrcVertices.Data[index + 0U]); index += 1U;
	vertex.BoneWeight[2] = decodeFloat(SrcVertices.Data[index + 0U]); index += 1U;
	vertex.BoneWeight[3] = decodeFloat(SrcVertices.Data[index + 0U]); index += 1U;

	return vertex;
}

void encodeFloat(float v0, uint index)
{
	DstVertices.Data[index] = floatBitsToUint(v0);
}

void encodeVec2(vec2 v0, uint index)
{
	encodeFloat(v0.x, index + 0U);
	encodeFloat(v0.y, index + 1U);
}

void encodeVec3(vec3 v0, uint index)
{
	encodeFloat(v0.x, index + 0U);
	encodeFloat(v0.y, index + 1U);
	encodeFloat(v0.z, index + 2U);
}

void encodeDestination(in MeshVertex vertex, uint index)
{
	encodeVec3(vertex.Position, index); index += 3U;
	encodeVec2(vertex.TexCoords, index); index += 2U;
	encodeVec3(vertex.ModelNormal, index); index += 3U;
	encodeVec3(vertex.ModelTangent, index); index += 3U;
	encodeVec3(vertex.ModelBitangent, index); index += 3U;
}

void main() {
	uint byteOffset = gl_GlobalInvocationID.x * 22U;
	
	SkinnedMeshVertex vertex = decodeSource(byteOffset);
	
	mat4 mat = mat4(0.0);
	
	
	if(vertex.BoneWeight[0] == 0.0 && vertex.BoneWeight[1] == 0.0 && vertex.BoneWeight[2] == 0.0 && vertex.BoneWeight[3] == 0.0)
	{
		mat = mat4(1.0);
	}
	else
	{
		for(uint i = 0U; i < 4U; ++i) 
		{
			if(vertex.BoneWeight[i] == 0.0)
				continue;
		
			mat += vertex.BoneWeight[i] * BoneTransforms.Data[vertex.BoneID[i]] * BoneOffsets.Data[vertex.BoneID[i]];
		}
	}

	MeshVertex dst;
	dst.Position = (mat * vec4(vertex.Position, 1.0)).xyz;
	dst.TexCoords = vertex.TexCoords;
	dst.ModelNormal = mat3(mat) * vertex.ModelNormal;
	dst.ModelTangent = mat3(mat) * vertex.ModelTangent;
	dst.ModelBitangent = mat3(mat) * vertex.ModelBitangent;
	
	encodeDestination(dst, gl_GlobalInvocationID.x * 14U);
}
#endif
