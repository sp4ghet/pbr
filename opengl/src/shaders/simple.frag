#version 460 core

out vec4 FragColor;

in vec2 uv;

uniform sampler2D texture1;
uniform sampler2D texture2;


void main(){
    FragColor = texture2D(texture2, uv) * texture2D(texture1, uv);
}
