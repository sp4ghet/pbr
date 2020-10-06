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

const int NUM_LIGHT_VERTS = 4;
uniform vec3 lightVerts[NUM_LIGHT_VERTS];
uniform bool ltcTwoSided;
uniform sampler2D ltc_mat;
uniform sampler2D ltc_mag;
uniform sampler2D lightCookie;

uniform bool hasDiffuse;
uniform bool hasNormal;
uniform bool hasRoughness;
uniform bool hasSpecular;

uniform vec3 lightColor;
uniform vec3 camPos;
uniform mat4 MVP;
uniform mat4 model;

uniform bool showNormal;
uniform bool showRoughness;

const float PI = 3.14159265;


float ClippedSphere(vec3 meanFlux){
    // Real-Time Area Lighting: a Journey From Research to Production (2016) p. 102
    // https://blog.selfshadow.com/publications/s2016-advances/s2016_ltc_rnd.pdf
    // Approximation of sphere clipping given the flux towards a sphere
    float l = length(meanFlux);
    return (l*l + meanFlux.z) / (l + 1.);
}

vec3 IntegrateEdge(vec3 v1, vec3 v2)
{
    float x = dot( v1, v2 );
    float y = abs( x );

    // Real-Time Area Lighting: a Journey From Research to Production (2016) p. 74
    // https://blog.selfshadow.com/publications/s2016-advances/s2016_ltc_rnd.pdf
    // a cubic fit to theta / sin(theta)
    float a = 0.8543985 + ( 0.4965155 + 0.0145206 * y ) * y;
    float b = 3.4175940 + ( 4.1616724 + y ) * y;
    float v = a / b;
    float theta_sintheta = ( x > 0.0 ) ? v : 0.5 * inversesqrt( max( 1.0 - x * x, 1e-7 ) ) - v;
    return cross( v1, v2 ) * theta_sintheta;
}

vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, bool twoSided){

    vec3 T1, T2;
    T1 = normalize( V - N * dot( V, N ) );
    T2 = - cross( N, T1 );

    // rotate area light in (T1, T2, N) basis
    Minv = Minv * transpose(mat3(T1, T2, N));

    vec3 L[4];
    L[0] = Minv * (lightVerts[0] - P);
    L[1] = Minv * (lightVerts[1] - P);
    L[2] = Minv * (lightVerts[2] - P);
    L[3] = Minv * (lightVerts[3] - P);
    // project onto sphere
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);

    // integrate
    vec3 flux = vec3(0.);

    flux += IntegrateEdge(L[0], L[1]);
    flux += IntegrateEdge(L[1], L[2]);
    flux += IntegrateEdge(L[2], L[3]);
    flux += IntegrateEdge(L[3], L[0]);

    float irradiance = ClippedSphere(flux);

    // check that we are on front face
    // if not, then invert winding order
    // https://github.com/mrdoob/three.js/blob/ebd5b3a3b370fb25e6f5f39153cb694c63ecc4b5/src/renderers/shaders/ShaderChunk/bsdfs.glsl.js#L202
    vec3 v1 = lightVerts[ 1 ] - lightVerts[ 0 ];
    vec3 v2 = lightVerts[ 3 ] - lightVerts[ 0 ];
    vec3 lightNormal = cross( v1, v2 );

    if( dot( lightNormal, P - lightVerts[ 0 ] ) < 0.0 ) irradiance = -irradiance;

    irradiance = twoSided ? abs(irradiance) : max(irradiance, 0.);

    vec3 Lo_i = vec3(irradiance);

    return Lo_i;
}

vec3 FetchDiffuseFilteredTexture(sampler2D cookie, vec3 f, vec3[4] L){
    vec3 V1 = (L[1] - L[0]);
    vec3 V2 = (L[3] - L[0]);
    vec3 n = cross(V1, V2);
    vec3 P = f * (dot(L[0], n) / dot(f, n)) - L[0];


    float dot_V1_V2 = dot(V1, V2);
    float inv_dot_V1_V1 = 1.0 / dot(V1, V1);
    vec3 V2_ = V2 - V1 * dot_V1_V2 * inv_dot_V1_V1;
    vec2 Puv;
    Puv.y = dot(V2_, P) / dot(V2_, V2_);
    Puv.x = dot(V1, P)*inv_dot_V1_V1 - dot_V1_V2*inv_dot_V1_V1*Puv.y ;

     // LOD
    float planeAreaSquared = dot(n, n);
    float planeDistxPlaneArea = dot(n, L[0]);
    float d = abs(planeDistxPlaneArea) / pow(planeAreaSquared, 0.75);
    float lod = log(2048.0*d)/log(3.0);

    vec2 uv = clamp(Puv, vec2(0.0), vec2(1.0));

    return textureLod(cookie, uv, lod).rgb;
}

vec3 LTC_EvaluateWithTexture(sampler2D cookie, vec3 N, vec3 V, vec3 P, mat3 Minv, bool twoSided){

    vec3 T1, T2;
    T1 = normalize( V - N * dot( V, N ) );
    T2 = - cross( N, T1 );

    // rotate area light in (T1, T2, N) basis
    Minv = Minv * transpose(mat3(T1, T2, N));

    // light vertices in cosine lobe space
    vec3 LC[4];
    LC[0] = Minv * (lightVerts[0] - P);
    LC[1] = Minv * (lightVerts[1] - P);
    LC[2] = Minv * (lightVerts[2] - P);
    LC[3] = Minv * (lightVerts[3] - P);

    // light vertices in cosine lobe space projected to hemisphere
    vec3 LH[4];
    LH[0] = normalize(LC[0]);
    LH[1] = normalize(LC[1]);
    LH[2] = normalize(LC[2]);
    LH[3] = normalize(LC[3]);

    // integrate
    vec3 flux = vec3(0.);

    flux += IntegrateEdge(LH[0], LH[1]);
    flux += IntegrateEdge(LH[1], LH[2]);
    flux += IntegrateEdge(LH[2], LH[3]);
    flux += IntegrateEdge(LH[3], LH[0]);

    // check that we are on front face
    // if not, then invert winding order
    // https://github.com/mrdoob/three.js/blob/ebd5b3a3b370fb25e6f5f39153cb694c63ecc4b5/src/renderers/shaders/ShaderChunk/bsdfs.glsl.js#L202
    vec3 v1 = lightVerts[ 1 ] - lightVerts[ 0 ];
    vec3 v2 = lightVerts[ 3 ] - lightVerts[ 0 ];
    vec3 lightNormal = cross( v1, v2 );
    if(twoSided){
        if(dot( lightNormal, P - lightVerts[ 0 ] ) < 0.0 ){
            flux = -flux;
        }
    }else{
        if(dot( lightNormal, P - lightVerts[ 0 ] ) < 0.0 ){
            return vec3(0.);
        }
    }

    vec3 textureLight = FetchDiffuseFilteredTexture(cookie, flux, LC);

    float irradiance = ClippedSphere(flux);

    irradiance = max(irradiance, 0.);

    vec3 Lo_i = vec3(irradiance) * textureLight;

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

    vec3 spec = LTC_EvaluateWithTexture(lightCookie, n, v, pos, Minv, ltcTwoSided);

    float norm = texture(ltc_mag, uv).r;
    spec *= norm;

    vec3 diff = LTC_EvaluateWithTexture(lightCookie, n, v, pos, mat3(1.), ltcTwoSided);

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
