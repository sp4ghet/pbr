#version 460 core

out vec4 FragColor;

in VS_OUT{
  vec3 vPos;
  vec2 uv;
  mat3 TBN;
} fs_in;


uniform vec3 color;

void main(){
    vec3 c = vec3(color);

    FragColor = vec4(c, 1.);
}
