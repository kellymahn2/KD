#ifdef comp
#version 460 core

layout(local_size_x = 16, local_size_y = 9,local_size_z = 4) in;

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
 
 //Output for indices.
layout(set = 1,binding = 1,std430) buffer GridSSBO{
	ClusterGrid Grids[];
};

layout(push_constant) uniform PushConstants {
	mat4 ViewMatrix;
	uint LightCount;
};

bool testHitSphere(uint light,uint tileIndex);
float sqDistPointAABB(vec3 point, uint tile);
void main(){
	uint lightCount = LightCount;

	 // Calculate tileIndex based on global invocation ID
    uint globalX = gl_GlobalInvocationID.x;
    uint globalY = gl_GlobalInvocationID.y;
    uint globalZ = gl_GlobalInvocationID.z;

	uint totalX = 16;
    uint totalY = 9;

    uint tileIndex = globalZ * (totalX * totalY) + globalY * totalX + globalX;
	
	uint clusterLightCount = 0;

	for(uint lightIndex = 0;lightIndex < lightCount && clusterLightCount < 50;++lightIndex){
		if(testHitSphere(lightIndex,tileIndex)){
			Grids[tileIndex].Indices[clusterLightCount] = lightIndex;
			++clusterLightCount;
		}
	}

	Grids[tileIndex].Count = clusterLightCount;
}

bool testHitSphere(uint light,uint tileIndex){
	float radius = Lights[light].Radius;
	vec3 center = vec3(ViewMatrix * Lights[light].Position);
	float distanceSquared = sqDistPointAABB(center,tileIndex);

	return distanceSquared <= (radius * radius);
}

float sqDistPointAABB(vec3 point, uint tile){
    float sqDist = 0.0;
    VolumeTileAABB currentCell = Clusters[tile].AABB;
    for(int i = 0; i < 3; ++i){
        float v = point[i];
        if(v < currentCell.Min[i]){
            sqDist += (currentCell.Min[i] - v) * (currentCell.Min[i] - v);
        }
        if(v > currentCell.Max[i]){
            sqDist += (v - currentCell.Max[i]) * (v - currentCell.Max[i]);
        }
    }

    return sqDist;
}
