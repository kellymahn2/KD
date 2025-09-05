#version 460 core
#ifdef vertex

layout(location = 0) in vec3 a_Position;

layout(push_constant) uniform PushConstants
{
    mat4 ViewProjection;
    mat4 Model;
    vec4 Color;
    vec2 ScreenSize;
    float WireframeWidth;
};

void main()
{
    gl_Position = ViewProjection * Model * vec4(a_Position, 1.0);
}
#endif

#ifdef geo
layout(triangles, invocations = 1) in;
layout(triangle_strip, max_vertices = 3) out;

layout(push_constant) uniform PushConstants
{
    mat4 ViewProjection;
    mat4 Model;
    vec4 Color;
    vec2 ScreenSize;
    float WireframeWidth;
};


layout(location = 0) noperspective out vec4 V0;
layout(location = 1) noperspective out vec4 V1;
layout(location = 2) out vec4 Pos;
layout(location = 3) flat out uint Case;

const uvec4 ProjectionCases[8] = 
{ 
  uvec4(0, 1, 2, 2),
  uvec4(0, 1, 2, 2), 
  uvec4(0, 2, 1, 1),
  uvec4(0, 0, 1, 2),
  uvec4(1, 2, 0, 0),
  uvec4(1, 1, 0, 2),
  uvec4(2, 2, 0, 1),
  uvec4(0, 0, 0, 0),
};


float HeronArea(float a, float b, float c)
{
    float S = (a + b + c) * 0.5;
    float val = S * (S - a) * (S - b) * (S - c);
    val = sqrt(val);

    return val;
}

// From window pixel pos to projection frame at the specified z (view frame). 
vec2 projToWindow(in vec4 pos)
{
    return vec2(  ScreenSize.x*0.5*((pos.x/pos.w) + 1),
                    ScreenSize.y*0.5*(1-(pos.y/pos.w)));
}

void main() {  
    vec4 p;

    uint indexOffset = 0;

    p = gl_in[0].gl_Position;
    vec2 p0 = projToWindow(p);
    
    if(p.z < 0.0)
        indexOffset += 4;


    p = gl_in[1].gl_Position;
    vec2 p1 = projToWindow(p);

    if(p.z < 0.0)
        indexOffset += 2;
    
    p = gl_in[2].gl_Position;
    vec2 p2 = projToWindow(p);


    if(p.z < 0.0)
        indexOffset += 1;
    
    vec2 points[3] = {p0, p1, p2};

    vec4 v0[3];
    vec4 v1[3];
    
    if(indexOffset == 0)
    {
        float a = length(p1 - p2);
        float b = length(p2 - p0);
        float c = length(p1 - p0);

        float area = HeronArea(a, b, c);

        float ha = 2.0 * area / a;
        float hb = 2.0 * area / b;
        float hc = 2.0 * area / c;

        v0[0] = vec4(ha, 0.0, 0.0, 0.0);
        v0[1] = vec4(0.0, hb, 0.0, 0.0);
        v0[2] = vec4(0.0, 0.0, hc, 0.0);
        v1[0] = vec4(0.0);
        v1[1] = vec4(0.0);
        v1[2] = vec4(0.0);
    }
    else 
    {
        vec4 edgeA, edgeB;
        edgeA.xy = points[ ProjectionCases[indexOffset].x ];
        edgeB.xy = points[ ProjectionCases[indexOffset].y ];

		edgeA.zw = normalize( edgeA.xy - points[ ProjectionCases[indexOffset].z ] ); 
        edgeB.zw = normalize( edgeB.xy - points[ ProjectionCases[indexOffset].w ] );

        v0[0] = edgeA;
        v0[1] = edgeA;
        v0[2] = edgeA;
        v1[0] = edgeB;
        v1[1] = edgeB;
        v1[2] = edgeB;
    }

    gl_Position = gl_in[0].gl_Position;
    Pos = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
    V0 = v0[0];
    V1 = v1[0];
    Case = indexOffset;
    EmitVertex();    

    gl_Position = gl_in[1].gl_Position;
    Pos = gl_in[1].gl_Position / gl_in[1].gl_Position.w;
    V0 = v0[1];
    V1 = v1[1];
    Case = indexOffset;
    EmitVertex();    

    gl_Position = gl_in[2].gl_Position;
    Pos = gl_in[2].gl_Position / gl_in[2].gl_Position.w;
    V0 = v0[2];
    V1 = v1[2];
    Case = indexOffset;
    EmitVertex();    

    EndPrimitive();
}  

#endif

#ifdef fragment

layout(location = 0) out vec4 o_Color;

layout(push_constant) uniform PushConstants
{
    mat4 ViewProjection;
    mat4 Model;
    vec4 Color;
    vec2 ScreenSize;
    float WireframeWidth;
};

layout(location = 0) noperspective in vec4 V0;
layout(location = 1) noperspective in vec4 V1;
layout(location = 2) in vec4 Pos;
layout(location = 3) flat in uint Case;


float GetDistance()
{
    float dist;

    if (Case == 0)
    {
        dist = min(min(V0.x, V0.y), V0.z);
    }
    else
    {
        vec2 AF = Pos.xy - V0.xy;
        float sqAF = dot(AF,AF);
        float AFcosA = dot(AF, V0.zw);
        dist = abs(sqAF - AFcosA*AFcosA);

        vec2 BF = Pos.xy - V1.xy;
        float sqBF = dot(BF,BF);
        float BFcosB = dot(BF, V1.zw);
        dist = min( dist, abs(sqBF - BFcosB*BFcosB) );
       
        // Only need to care about the 3rd edge for some cases.
        if (Case == 1 || Case == 2 || Case == 4)
        {
            float AFcosA0 = dot(AF, normalize(V1.xy - V0.xy));
			dist = min( dist, abs(sqAF - AFcosA0*AFcosA0) );
	    }

        dist = sqrt(dist);
    }

    return dist;
}

void main()
{
    float d = GetDistance();

    float mixVal = 0.0;

    float width = WireframeWidth;

    if(d < width - 1)
    {
        mixVal = 1.0;
    }
    else if(d > width + 1)
    {
        discard;
    }
    else
    {
        float x = d - (width - 1);
        mixVal = exp2(-2.0 * x * x);
    }



    o_Color = mix(vec4(0.0), Color, mixVal);
}
#endif
