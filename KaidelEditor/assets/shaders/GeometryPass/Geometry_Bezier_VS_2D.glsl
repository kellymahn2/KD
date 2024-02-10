#version 460 core
layout(location = 0) in vec3 a_Position;

layout(location = 0)out vec3 v_VSPosition;

void main(){
    v_VSPosition = a_Position;
    gl_Position =  vec4(a_Position,1.0);
}