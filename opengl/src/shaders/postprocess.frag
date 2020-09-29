#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D renderBuffer;

void main(){
    vec3 c = vec3(1.);
    c = texture(renderBuffer, TexCoords).rgb;

    FragColor = vec4(c, 1.);

}
