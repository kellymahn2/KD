#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in float a_LineWidth;

layout(push_constant) uniform PushConstants{
	mat4 ViewProjection;
};


layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out float v_LineWidth;


void main(){
	gl_Position = vec4(a_Position, 1.0);
	//gl_Position.y = -gl_Position.y;
	v_Color = a_Color;
	v_Normal = normalize(a_Normal);
	v_LineWidth = a_LineWidth;
}

#type geometry
#version 460 core

layout(lines) in;
layout(triangle_strip,max_vertices = 6) out;

layout(push_constant) uniform PushConstants{
	mat4 ViewProjection;
};

layout(location = 0) in vec4 v_Color[];
layout(location = 1) in vec3 v_Normal[];
layout(location = 2) in float v_LineWidth[];

layout(location = 0) out vec4 v_FragColor;
layout(location = 1) out float v_Distance;

void main(){
	vec3 p0 = gl_in[0].gl_Position.xyz;
	vec3 p1 = gl_in[1].gl_Position.xyz;

	vec3 dir0 = v_Normal[0] * v_LineWidth[0] * 1.1;
	vec3 dir1 = v_Normal[1] * v_LineWidth[0] * 1.1;

	vec3 o0 = p0 + dir0;
	vec3 o1 = p0 - dir0;
	vec3 o2 = p1 - dir1;
	vec3 o3 = p1 + dir1;

	gl_Position = ViewProjection * vec4(o0, 1.0);
	gl_Position.y = -gl_Position.y;
	v_FragColor = v_Color[0];
	v_Distance = 0.5;
	EmitVertex();
	
	gl_Position = ViewProjection * vec4(o1, 1.0);
	gl_Position.y = -gl_Position.y;
	v_FragColor = v_Color[0];
	v_Distance = -0.5;
	EmitVertex();
	
	gl_Position = ViewProjection * vec4(o2, 1.0);
	gl_Position.y = -gl_Position.y;
	v_FragColor = v_Color[1];
	v_Distance = -0.5;
	EmitVertex();

	EndPrimitive();
	
	gl_Position = ViewProjection * vec4(o2, 1.0);
	gl_Position.y = -gl_Position.y;
	v_FragColor = v_Color[1];
	v_Distance = -0.5;
	EmitVertex();

	gl_Position = ViewProjection * vec4(o3, 1.0);
	gl_Position.y = -gl_Position.y;
	v_FragColor = v_Color[1];
	v_Distance = 0.5;
	EmitVertex();

	gl_Position = ViewProjection * vec4(o0, 1.0);
	gl_Position.y = -gl_Position.y;
	v_FragColor = v_Color[0];
	v_Distance = 0.5;
	EmitVertex();

	EndPrimitive();
}


#type fragment
#version 460 core

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec4 v_FragColor;

void main(){
	o_Color = v_FragColor;
}
