#version 460 core

out vec4 FragColor;

in vec2 uv;

uniform sampler2D texture_diffuse1;
uniform vec3 fillColor;

void main(){
    FragColor = vec4(fillColor, 1.);
}
