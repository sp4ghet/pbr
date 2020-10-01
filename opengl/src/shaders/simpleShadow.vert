#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 st;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 vPos;

uniform mat4 MVP;
uniform mat4 model;
uniform mat4 lightVP;

void main()
{
    mat4 lightMVP = lightVP * model;
    gl_Position = lightMVP * vec4(aPos, 1.);
    vPos = gl_Position.xyz;
}
