#type vertex
#version 460 core

layout(location = 0) in vec2 a_NDC;
layout(location = 1) in vec2 a_TexCoords;

layout(location = 0)out vec2 v_TexCoords;

void main(){
	gl_Position = vec4(a_NDC,0.0,1.0);
	gl_Position.y *= -1.0;
	v_TexCoords = a_TexCoords;
}
#type fragment
#version 460 core

const int SplitCount = 4;
const int PCFSamples = 9;
const int ShadowMapSize = 2048;

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
layout(set = 2,binding = 3) uniform texture2D Albedo;
layout(set = 2,binding = 4) uniform texture2D MetallicRoughness;
layout(set = 2,binding = 5) uniform texture2D Depths;

layout(set = 3,binding = 0, std140) uniform DirLight{
	mat4 ViewProjection[4];
	vec3 Direction;
	vec3 Color;
	vec4 SplitDistances;
} DLight;
layout(set = 3,binding = 1) uniform sampler ShadowSampler;
layout(set = 3,binding = 2) uniform texture2D DirectionalShadows0;
layout(set = 3,binding = 3) uniform texture2D DirectionalShadows1;
layout(set = 3,binding = 4) uniform texture2D DirectionalShadows2;
layout(set = 3,binding = 5) uniform texture2D DirectionalShadows3;



const float M_PI = 3.1415926535897932384626433832795;

layout(push_constant) uniform PushConstants{
	mat4 View;
	vec3 CameraPos;
	vec2 ScreenDimensions;
	float zNear;
	float zFar;
	float SliceScale;
	float SliceBias;
	int ShowDebugShadow;
};


vec3 PointLight(vec3 pos,vec3 norm,vec3 viewDir,uint lightIndex);
vec3 Calculate(vec3 pos,vec3 norm,vec3 viewDir,uint tileIndex);
float linearDepth(float depthSample);

//PBR Functions
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
float distributionGGX(vec3 N, vec3 H, float rough);
float geometrySchlickGGX(float nDotV, float rough);
float geometrySmith(float nDotV, float nDotL, float rough);

float calcDirShadow(vec3 worldPos,vec3 norm,vec3 lightDir);

vec3 calcDirLight(float shadow, vec3 direction, vec3 color, vec3 normal, 
					vec3 viewDir, vec3 albedo, float rough, 
					float metal,vec3 F0);

vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos,
                    vec3 viewDir, vec3 albedo, float rough,
                    float metal, vec3 F0,  float viewDistance);
uint CalcTileIndex(float ndcDepth){
	uint zTile = uint(max(log2(linearDepth(ndcDepth)) * SliceScale + SliceBias, 0.0));
	vec2 clusterCoord = vec2(16,9) * (gl_FragCoord.xy / ScreenDimensions);
	uvec2 cluster2DCoord = clamp(uvec2(clusterCoord),uvec2(0,0),uvec2(15,8));
	uvec3 tiles = uvec3(cluster2DCoord,zTile);
	uint tileIndex = tiles.x +
                     16 * tiles.y +
                     ( 16 * 9) * tiles.z; 
	return tileIndex;
}

void main(){
	float ndcDepth = texture(sampler2D(Depths,GlobalSampler),v_TexCoords).r;
	
	if(ndcDepth == 1.0){
		discard;
	}

	vec3 pos = vec3(texture(sampler2D(Positions,GlobalSampler), v_TexCoords));
	vec3 norm = vec3(texture(sampler2D(Normals,GlobalSampler), v_TexCoords));
	vec4 color = texture(sampler2D(Albedo,GlobalSampler),v_TexCoords);
	
	//if(color.a != 1.0){
	//	discard;
	//}

	vec2 mr = texture(sampler2D(MetallicRoughness,GlobalSampler),v_TexCoords).rg;
	float metallic = mr.r;
	float roughness = mr.g;
	
	vec3 viewDir = normalize(CameraPos - pos);
	
	vec3 tileSize = vec3(ScreenDimensions, zFar - zNear) / vec3(16,9,24);
	
	uint tileIndex = CalcTileIndex(ndcDepth);
	
	vec3 F0 = vec3(0.04);
	F0 = mix(F0,color.rgb,metallic);

	float viewDistance = length(CameraPos - pos);

	vec3 radiance = vec3(0.0);

	radiance = calcDirLight(calcDirShadow(pos,norm,-normalize(DLight.Direction)), DLight.Direction,DLight.Color,norm,viewDir,color.rgb,roughness,metallic,F0);

	//for(uint i = 0;i< Grids[tileIndex].Count;++i) {
	//	uint lightIndex = Grids[tileIndex].Indices[i];
	//	radiance += calcPointLight(lightIndex,norm,pos,viewDir,color.rgb,roughness,metallic,F0,viewDistance);
	//}

	vec3 ambient = vec3(0.025) * color.rgb;

	radiance += ambient;
	//radiance = vec3(calcDirShadow(pos),0.0,0.0);

	if(ShowDebugShadow == 0)
		o_Color = vec4(radiance,1.0);
}

float linearDepth(float depthSample){
    float depthRange = 2.0 * depthSample - 1.0;
    // Near... Far... wherever you are...
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
    return linear;
}


float PCFSample(in texture2D shadowTexture,vec3 baseCoords,float bias){
	float scale = 1.0;
	vec2 texelSize = scale * 1.0 / vec2(textureSize(sampler2D(shadowTexture,ShadowSampler),0));
	
	float shadow = 0.0;

	int radius = 1;

	float count = 0.0;

	for(int i = -radius;i<=radius;++i){
		for(int j = -radius;j<=radius;++j){
			vec2 offset = vec2(float(i) * texelSize.x,float(j) * texelSize.y);
			float pcfDepth = texture(sampler2D(shadowTexture,ShadowSampler),baseCoords.xy + offset).r;

			shadow += (baseCoords.z - bias) > pcfDepth ? 1.0 : 0.0;
			count += 1.0;
		}
	}

	shadow /= count;
	return shadow;
}

bool IsOutOfBounds(vec4 coords){
	return 
	(coords.x < -coords.w || coords.x > coords.w) ||
	(coords.y < -coords.w || coords.y > coords.w) ||
	(coords.z < 0.0 || coords.z > coords.w);
}


float calcDirShadow(vec3 worldPos,vec3 norm,vec3 lightDir)  
{

	vec4 fragPosViewSpace = View * vec4(worldPos,1.0);
	fragPosViewSpace /= fragPosViewSpace.w;

	float viewDepth = abs(fragPosViewSpace.z);

	int index = -1;

	for(int i = 0;i<4;i++){
		if(viewDepth < DLight.SplitDistances[i]){
			index = i;
			break;
		}
	}

	if(index == -1)
		return 0.0;

	if(ShowDebugShadow == 4){
		switch(index){
		case 0:o_Color = vec4(1,0,0,1);break;
		case 1:o_Color = vec4(0,1,0,1);break;
		case 2:o_Color = vec4(0,0,1,1);break;
		case 3:o_Color = vec4(1,1,1,1);break;
		}
		return 0.0;
	}

	vec4 fragPosLightSpace = DLight.ViewProjection[index] * vec4(worldPos,1.0);
	
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	if(ShowDebugShadow == 1){
		if(projCoords.x > 1.0 || projCoords.x < -1.0)
		{
			o_Color = vec4(0,0,1,1);
		}
		else if(projCoords.y > 1.0 || projCoords.y < -1.0)
		{
			o_Color = vec4(1,1,0,1);
		}
		else if(projCoords.z > 1.0 || projCoords.z < 0.0)
		{
			o_Color = vec4(1,0,1,1);
		}
		else {
			o_Color = vec4(1);
		}
	}

	if(IsOutOfBounds(fragPosLightSpace))
		return 0.0;

	projCoords.xy = projCoords.xy * 0.5 + 0.5;
	
	float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);
	//bias = 0.005;
	float shadow = 0.0;
	if(index == 0)
		shadow = PCFSample(DirectionalShadows0, projCoords,bias);
	if(index == 1)
		shadow = PCFSample(DirectionalShadows1, projCoords,bias);
	if(index == 2)
		shadow = PCFSample(DirectionalShadows2, projCoords,bias);
	if(index == 3)
		shadow = PCFSample(DirectionalShadows3, projCoords,bias);


	if(ShowDebugShadow == 2){	
		if(shadow != 0.0)
		{
			o_Color = vec4(1,0,0,1);
		}
		else{
			o_Color = vec4(0,1,0,1);
		}
	}

	if(ShowDebugShadow == 3){
		o_Color = vec4(shadow);
	}
	
	return shadow;
} 

vec3 calcDirLight(float shadow, vec3 direction, vec3 color, vec3 normal, 
					vec3 viewDir, vec3 albedo, float rough, 
					float metal, vec3 F0){
    //Variables common to BRDFs
    vec3 lightDir = normalize(-direction);
    vec3 halfway  = normalize(lightDir + viewDir);
    float nDotV = max(dot(normal, viewDir), 0.0);
    float nDotL = max(dot(normal, lightDir), 0.0);
    vec3 radianceIn = color;

    //Cook-Torrance BRDF
    float NDF = distributionGGX(normal, halfway, rough);
    float G   = geometrySmith(nDotV, nDotL, rough);
    vec3  F   = fresnelSchlick(max(dot(halfway,viewDir), 0.0), F0);

    //Finding specular and diffuse component
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * nDotV * nDotL;
    vec3 specular = numerator / max (denominator, 0.0001);

    vec3 radiance = (kD * (albedo / M_PI) + specular ) * radianceIn * nDotL;
	radiance = (1.0 - shadow) * radiance;

    return radiance;
}

vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos,
                    vec3 viewDir, vec3 albedo, float rough,
                    float metal, vec3 F0,  float viewDistance){
    //Point light basics
	vec3 position = Lights[index].Position.xyz;
    vec3 color    = 100.0 * Lights[index].Color.rgb;
    float radius  = Lights[index].Radius;

    //Stuff common to the BRDF subfunctions 
    vec3 lightDir = normalize(position - fragPos);
    vec3 halfway  = normalize(lightDir + viewDir);
    float nDotV = max(dot(normal, viewDir), 0.0);
    float nDotL = max(dot(normal, lightDir), 0.0);

    //Attenuation calculation that is applied to all
    float distance    = length(position - fragPos);
    float attenuation = pow(clamp(1 - pow((distance / radius), 4.0), 0.0, 1.0), 2.0)/(1.0  + (distance * distance));
    vec3 radianceIn   = color * attenuation;

    //Cook-Torrance BRDF
    float NDF = distributionGGX(normal, halfway, rough);
    float G   = geometrySmith(nDotV, nDotL, rough);
    vec3  F   = fresnelSchlick(max(dot(halfway,viewDir), 0.0), F0);

    //Finding specular and diffuse component
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * nDotV * nDotL;
    vec3 specular = numerator / max(denominator, 0.0000001);
    // vec3 specular = numerator / denominator;

    vec3 radiance = (kD * (albedo / M_PI) + specular ) * radianceIn * nDotL;

    return radiance;
}


// PBR functions
vec3 fresnelSchlick(float cosTheta, vec3 F0){
    float val = 1.0 - cosTheta;
    return F0 + (1.0 - F0) * (val*val*val*val*val); //Faster than pow
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness){
    float val = 1.0 - cosTheta;
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * (val*val*val*val*val); //Faster than pow
}

float distributionGGX(vec3 N, vec3 H, float rough){
    float a  = rough * rough;
    float a2 = a * a;

    float nDotH  = max(dot(N, H), 0.0);
    float nDotH2 = nDotH * nDotH;

    float num = a2; 
    float denom = (nDotH2 * (a2 - 1.0) + 1.0);
    denom = 1 / (M_PI * denom * denom);

    return num * denom;
}

float geometrySchlickGGX(float nDotV, float rough){
    float r = (rough + 1.0);
    float k = r*r / 8.0;

    float num = nDotV;
    float denom = 1 / (nDotV * (1.0 - k) + k);

    return num * denom;
}

float geometrySmith(float nDotV, float nDotL, float rough){
    float ggx2  = geometrySchlickGGX(nDotV, rough);
    float ggx1  = geometrySchlickGGX(nDotL, rough);

    return ggx1 * ggx2;
}
