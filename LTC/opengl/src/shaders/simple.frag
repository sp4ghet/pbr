#version 460 core

out vec4 FragColor;

in VS_OUT{
  vec3 vPos;
  vec2 uv;
  mat3 TBN;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1; // metallic
uniform sampler2D texture_roughness1; // roughness
uniform sampler2D texture_normal1;

uniform float roughness;

uniform sampler2D ltc_mat;
uniform sampler2D ltc_mag;

uniform bool hasDiffuse;
uniform bool hasNormal;
uniform bool hasRoughness;
uniform bool hasSpecular;

const int NUM_LIGHT_VERTS = 4;
uniform vec3 lightVerts[NUM_LIGHT_VERTS];

uniform vec3 lightColor;
uniform vec3 camPos;
uniform mat4 MVP;
uniform mat4 model;

uniform bool showNormal;
uniform bool showRoughness;

const float PI = 3.14159265;



float IntegrateEdge(vec3 v1, vec3 v2)
{
    float cosTheta = dot(v1, v2);
    float theta = acos(cosTheta);
    float res = cross(v1, v2).z * ((theta > 0.001) ? theta/sin(theta) : 1.0);

    return res;
}

void ClipQuadToHorizon(inout vec3 L[5], out int n)
{
    // detect clipping config
    int config = 0;
    if (L[0].z > 0.0) config += 1;
    if (L[1].z > 0.0) config += 2;
    if (L[2].z > 0.0) config += 4;
    if (L[3].z > 0.0) config += 8;

    // clip
    n = 0;

    if (config == 0)
    {
        // clip all
    }
    else if (config == 1) // V1 clip V2 V3 V4
    {
        n = 3;
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
        L[2] = -L[3].z * L[0] + L[0].z * L[3];
    }
    else if (config == 2) // V2 clip V1 V3 V4
    {
        n = 3;
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
    }
    else if (config == 3) // V1 V2 clip V3 V4
    {
        n = 4;
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
        L[3] = -L[3].z * L[0] + L[0].z * L[3];
    }
    else if (config == 4) // V3 clip V1 V2 V4
    {
        n = 3;
        L[0] = -L[3].z * L[2] + L[2].z * L[3];
        L[1] = -L[1].z * L[2] + L[2].z * L[1];
    }
    else if (config == 5) // V1 V3 clip V2 V4) impossible
    {
        n = 0;
    }
    else if (config == 6) // V2 V3 clip V1 V4
    {
        n = 4;
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
        L[3] = -L[3].z * L[2] + L[2].z * L[3];
    }
    else if (config == 7) // V1 V2 V3 clip V4
    {
        n = 5;
        L[4] = -L[3].z * L[0] + L[0].z * L[3];
        L[3] = -L[3].z * L[2] + L[2].z * L[3];
    }
    else if (config == 8) // V4 clip V1 V2 V3
    {
        n = 3;
        L[0] = -L[0].z * L[3] + L[3].z * L[0];
        L[1] = -L[2].z * L[3] + L[3].z * L[2];
        L[2] =  L[3];
    }
    else if (config == 9) // V1 V4 clip V2 V3
    {
        n = 4;
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
        L[2] = -L[2].z * L[3] + L[3].z * L[2];
    }
    else if (config == 10) // V2 V4 clip V1 V3) impossible
    {
        n = 0;
    }
    else if (config == 11) // V1 V2 V4 clip V3
    {
        n = 5;
        L[4] = L[3];
        L[3] = -L[2].z * L[3] + L[3].z * L[2];
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
    }
    else if (config == 12) // V3 V4 clip V1 V2
    {
        n = 4;
        L[1] = -L[1].z * L[2] + L[2].z * L[1];
        L[0] = -L[0].z * L[3] + L[3].z * L[0];
    }
    else if (config == 13) // V1 V3 V4 clip V2
    {
        n = 5;
        L[4] = L[3];
        L[3] = L[2];
        L[2] = -L[1].z * L[2] + L[2].z * L[1];
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
    }
    else if (config == 14) // V2 V3 V4 clip V1
    {
        n = 5;
        L[4] = -L[0].z * L[3] + L[3].z * L[0];
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
    }
    else if (config == 15) // V1 V2 V3 V4
    {
        n = 4;
    }

    if (n == 3)
        L[3] = L[0];
    if (n == 4)
        L[4] = L[0];
}

vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv){
 vec3 T1, T2;
    T1 = normalize(V - N*dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, N) basis
    Minv = Minv * transpose(mat3(T1, T2, N));

    // polygon (allocate 5 vertices for clipping)
    vec3 L[5];
    L[0] = Minv * (lightVerts[0] - P);
    L[1] = Minv * (lightVerts[1] - P);
    L[2] = Minv * (lightVerts[2] - P);
    L[3] = Minv * (lightVerts[3] - P);

    int n;
    ClipQuadToHorizon(L, n);

    if (n == 0)
        return vec3(0, 0, 0);

    // project onto sphere
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);
    L[4] = normalize(L[4]);

    // integrate
    float sum = 0.0;

    sum += IntegrateEdge(L[0], L[1]);
    sum += IntegrateEdge(L[1], L[2]);
    sum += IntegrateEdge(L[2], L[3]);
    if (n >= 4)
        sum += IntegrateEdge(L[3], L[4]);
    if (n == 5)
        sum += IntegrateEdge(L[4], L[0]);

    sum = abs(sum);

    vec3 Lo_i = vec3(sum, sum, sum);

    return Lo_i;
}

void main(){
    vec3 c = vec3(0.);
    float metallic = hasSpecular ? texture(texture_specular1, fs_in.uv).r : 0.01;
    float alpha = hasRoughness  ? texture(texture_roughness1, fs_in.uv).r : roughness;
    vec3 albedo = hasDiffuse  ? texture(texture_diffuse1, fs_in.uv).rgb : vec3(0.8);
    vec3 normal = hasNormal ? texture(texture_normal1, fs_in.uv).rgb : vec3(0.5,0.5,1.);
    normal = normal * 2. - 1.; // remap [0,1] to [-1,1]
    normal = normalize(fs_in.TBN * normal);

    vec3 n = normal;
    vec3 v = normalize(camPos - fs_in.vPos);
    vec3 pos = fs_in.vPos;

    float theta = acos(dot(n,v));
    vec2 uv = vec2(alpha, theta/(0.5*PI));
    const float LUT_SIZE  = 64.0;
    const float LUT_SCALE = (LUT_SIZE - 1.0)/LUT_SIZE;
    const float LUT_BIAS  = 0.5/LUT_SIZE;
    uv = uv*LUT_SCALE + LUT_BIAS;
    vec4 t = texture(ltc_mat, uv);

    mat3 Minv = mat3(
      vec3(1., 0., t.y),
      vec3(0.,t.z, 0),
      vec3(t.w, 0., t.x)
    );

    vec3 spec = LTC_Evaluate(n, v, pos, Minv);

    float norm = texture(ltc_mag, uv).r;
    spec *= norm;

    vec3 diff = LTC_Evaluate(n, v, pos, mat3(1.));

    c = lightColor * (spec + albedo * diff);
    c /= 2. * PI;
    c += albedo * .1;

    if(showNormal && !showRoughness){
        c = normal;
    }
    if(showRoughness && !showNormal){
        c = vec3(1.) * alpha;
    }
    if(showNormal && showRoughness){
        c = normal * alpha;
    }

    FragColor = vec4(c, 1.);
}
