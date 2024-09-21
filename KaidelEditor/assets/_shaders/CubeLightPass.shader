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

struct VolumeTileAABB{
    vec4 Min;
    vec4 Max;
};

struct Light{
	vec4 Position;
	vec4 Color;
	float Radius;
};

struct Cluster{
	VolumeTileAABB AABB;
};

//Input for AABBs.
layout(set = 0,binding = 0,std430) buffer ClusterAABB{
	Cluster Clusters[];
};

//Input for lights.
layout(set = 1,binding = 0, std430) buffer LightSSBO{
	Light Lights[];
};

struct ClusterGrid{
	uint Count;
	uint Indices[50];
	uint Padding;
};
 
 //Input for indices.
layout(set = 1,binding = 1,std430) buffer GridSSBO{
	ClusterGrid Grids[];
};

layout(set = 2,binding = 0) uniform sampler GlobalSampler;
layout(set = 2,binding = 1) uniform texture2D Positions;
layout(set = 2,binding = 2) uniform texture2D Normals;
layout(set = 2,binding = 3) uniform texture2D AlbedoSpec;
layout(set = 2,binding = 4) uniform texture2D Depths;




layout(push_constant) uniform PushConstants{
	vec3 CameraPos;
	vec2 ScreenDimensions;
	float zNear;
	float zFar;
	float SliceScale;
	float SliceBias;
};

struct LightCalcResult{
	vec4 Ambient;
	vec4 Diffuse;
	vec4 Specular;
};

LightCalcResult PointLight(vec3 pos,vec3 norm,vec3 viewDir,uint lightIndex){
	LightCalcResult res;
	res.Ambient = vec4(0.0);
	res.Diffuse = vec4(0.0);
	res.Specular = vec4(0.0);

	vec3 position = Lights[lightIndex].Position.xyz;
    vec3 color    = Lights[lightIndex].Color.rgb;
    float radius  = Lights[lightIndex].Radius;

	vec3 lightDir = normalize(position - pos);
	vec3 halfway = normalize(lightDir + viewDir);
	float nDotV = max(dot(norm,viewDir),0.0);
	float nDotL = max(dot(norm,lightDir),0.0);

	float distance = length(position - pos);
	res.Ambient = vec4(color,1.0);
	res.Diffuse = vec4(color * nDotL,1.0);
    float spec = pow(max(dot(norm, halfway), 0.0), 32.0);
	res.Specular = vec4(color * spec,1.0);

	return res;
}

LightCalcResult Calculate(vec3 pos,vec3 norm,vec3 viewDir,uint tileIndex){
	LightCalcResult res;
	res.Ambient = vec4(0.0);
	res.Diffuse = vec4(0.0);
	res.Specular = vec4(0.0);
	for(uint i = 0;i< Grids[tileIndex].Count;++i) {
		LightCalcResult lightRes = PointLight(pos,norm,viewDir,i);
		res.Ambient += lightRes.Ambient;
		res.Diffuse += lightRes.Diffuse;
		res.Specular += lightRes.Specular;
	}

	return res;
}

float linearDepth(float depthSample){
    float depthRange = 2.0 * depthSample - 1.0;
    // Near... Far... wherever you are...
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
    return linear;
}

void main(){
	float ndcDepth = texture(sampler2D(Depths,GlobalSampler),v_TexCoords).r;
	
	//if(ndcDepth == 1.0){
	//	o_Color = vec4(1.0,.0,.0,1.0);
	//}

	vec3 pos = vec3(texture(sampler2D(Positions,GlobalSampler), v_TexCoords));
	vec3 norm = vec3(texture(sampler2D(Normals,GlobalSampler), v_TexCoords));
	vec4 albedoSpec = texture(sampler2D(AlbedoSpec,GlobalSampler),v_TexCoords);

	//vec3 viewDir = normalize(CameraPos - pos);
	//
	//vec3 tileSize = vec3(ScreenDimensions, zFar - zNear) / vec3(16,9,24);
	//
	//uint zTile = uint(max(log2(linearDepth(ndcDepth)) * SliceScale + SliceBias, 0.0));
	//vec2 clusterCoord = vec2(16,9) * (gl_FragCoord.xy / ScreenDimensions);
	//uvec2 cluster2DCoord = clamp(uvec2(clusterCoord),uvec2(0,0),uvec2(15,8));
	//uvec3 tiles = uvec3(cluster2DCoord,zTile);
	//uint tileIndex = tiles.x +
    //                 16 * tiles.y +
    //                 ( 16 * 9) * tiles.z; 
	//
	//LightCalcResult res = Calculate(pos,norm,viewDir,tileIndex);
	
	//1.0 should actually be the shadow value.
	vec3 color = 1.0 * (albedoSpec.rgb);

	o_Color = vec4(albedoSpec.rgb,1.0);
}
