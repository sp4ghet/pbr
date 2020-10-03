#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 2) in vec2 st;

out vec2 TexCoords;

void main(){
    gl_Position = vec4(pos, 1.);
    TexCoords = st;
}
