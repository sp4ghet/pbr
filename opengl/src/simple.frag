#version 460 core

out vec4 FragColor;
in vec4 vertexColor;
uniform vec4 ourColor;

void main(){
    FragColor = vertexColor + ourColor;
}
