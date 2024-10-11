#type vertex
#version 460 core
layout(location = 0)in vec3 a_Position;


layout(push_constant) uniform PushConstants{
	mat4 MVP;
};

void main(){
	gl_Position = MVP * vec4(a_Position,1.0);
}

#type fragment
#version 460 core

void main(){

}
