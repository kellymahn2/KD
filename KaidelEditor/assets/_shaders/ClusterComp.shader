#type comp
#version 460 core

#define MAX_LIGHTS_PER_CLUSTER 100
#define NUM_LIGHTS 100

layout(local_size_x = 1, local_size_y = 1) in;  // Work group size (1D cluster grid in Z)

struct VolumeTileAABB{
    vec4 Min;
    vec4 Max;
};

struct Cluster{
	VolumeTileAABB AABB;
};

layout(set = 0,binding = 0,std430) buffer ClusterAABB{
	Cluster Clusters[];
};

layout(push_constant) uniform PushConstants {
	mat4 InverseProjMatrix;
    float zNear;        
    float zFar;        
	uvec2 ScreenDimensions;
};


vec4 screen2View(vec4 screen);
vec3 lineIntersectionToZPlane(vec3 A, vec3 B, float zDistance);

void main() {

	const vec3 eyePos = vec3(0.0);
	vec3 tileSizes = vec3(ScreenDimensions,zFar - zNear) / vec3(gl_NumWorkGroups);

    uint clusterIndex = gl_WorkGroupID.x +
                     gl_WorkGroupID.y * gl_NumWorkGroups.x +
                     gl_WorkGroupID.z * (gl_NumWorkGroups.x * gl_NumWorkGroups.y);

	vec4 maxPointScreen = vec4(vec2((gl_WorkGroupID.x + 1) * tileSizes.x,(gl_WorkGroupID.y + 1)* tileSizes.y),-1.0,1.0);
	vec4 minPointScreen = vec4(vec2((gl_WorkGroupID.x) * tileSizes.x,(gl_WorkGroupID.y)* tileSizes.y),-1.0,1.0);

	vec3 maxPointView = screen2View(maxPointScreen).xyz;
	vec3 minPointView = screen2View(minPointScreen).xyz;

	float tileNear  = -zNear * pow(zFar/ zNear, gl_WorkGroupID.z/float(gl_NumWorkGroups.z));
    float tileFar   = -zNear * pow(zFar/ zNear, (gl_WorkGroupID.z + 1) /float(gl_NumWorkGroups.z));

	vec3 minPointNear = lineIntersectionToZPlane(eyePos, minPointView, tileNear );
    vec3 minPointFar  = lineIntersectionToZPlane(eyePos, minPointView, tileFar );
    vec3 maxPointNear = lineIntersectionToZPlane(eyePos, maxPointView, tileNear );
    vec3 maxPointFar  = lineIntersectionToZPlane(eyePos, maxPointView, tileFar );

	vec3 minPointAABB = min(min(minPointNear, minPointFar),min(maxPointNear, maxPointFar));
    vec3 maxPointAABB = max(max(minPointNear, minPointFar),max(maxPointNear, maxPointFar));

	Clusters[clusterIndex].AABB.Min = vec4(minPointAABB,0.0);
	Clusters[clusterIndex].AABB.Max = vec4(maxPointAABB,0.0);
}

vec4 screen2View(vec4 screen){
    //Convert to NDC
    vec2 texCoord = screen.xy / ScreenDimensions.xy;

    //Convert to clipSpace
    vec4 clip = vec4(vec2(texCoord.x, 1.0 - texCoord.y)* 2.0 - 1.0, screen.z, screen.w);

    //View space transform
    vec4 view = InverseProjMatrix * clip;

    //Perspective projection
    view = view / view.w;

    return view;
}

vec3 lineIntersectionToZPlane(vec3 A, vec3 B, float zDistance){
    //Because this is a Z based normal this is fixed
    vec3 normal = vec3(0.0, 0.0, 1.0);

    vec3 ab =  B - A;

    //Computing the intersection length for the line and the plane
    float t = (zDistance - dot(normal, A)) / dot(normal, ab);

    //Computing the actual xyz position of the point along the line
    vec3 result = A + t * ab;

    return result;
}
