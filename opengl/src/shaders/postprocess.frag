#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D renderBuffer;

const float offset = 1.0 / 300.0;

vec2 offsets[9] = vec2[](
    vec2(-offset, offset),
    vec2( 0.0f, offset),
    vec2( offset,offset),
    vec2(-offset, 0.0f),
    vec2( 0.0f, 0.0f),
    vec2( offset, 0.0f),
    vec2(-offset, -offset),
    vec2( 0.0f, -offset),
    vec2( offset, -offset)
);


// sobel kernel
float kernel[9] =float[](
    1.0 / 16, 1.0/ 16, 1.0 / 16,
    1.0 / 16, -8.0/ 16, 1.0 / 16,
    1.0 / 16, 1.0/ 16, 1.0 / 16
);


vec3 convolve(){
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(renderBuffer, TexCoords + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

    return col;
}

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float near_plane = 1.; float far_plane = 15.;
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main(){
    vec3 c = vec3(1.);
    c  = texture(renderBuffer, TexCoords).rgb;

    c = smoothstep(vec3(0.), vec3(1.), c);
    c = pow(c, vec3(0.4545));

    FragColor = vec4(c, 1.);
}
