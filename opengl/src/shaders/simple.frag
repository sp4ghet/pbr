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

uniform vec3 camPos;
uniform mat4 MVP;

const float PI = 3.14159265;

// Eq 9.36 from Physically Based Rendering Vol 4.
vec3 diffuse(vec3 rho, vec3 f0, vec3 n, vec3 l, vec3 v){
  vec3 c = rho;

  float nl = max(0., dot(n,l)), nv = max(0., dot(n,v));

  c *= (21./20.)*(1. - pow(1.-nl,5.))*(1. - pow(1.-nv, 5.))*(1.- f0);
  return c;
}

float sq(float x){
  return x*x;
}

float g1Smith(vec3 h, vec3 v, float roughness){
  float dp = dot(h,v);
  float d = step(0., dp);
  // g(h,v,a) = chi+(dot(h,v)) / (1 + lambda(a))
  // lambda(a) = -.5 + .5 * sqrt(1 + 1/a^2)
  // a = 1 / roughness * tan(th)
  // tan^2(th) = 1 / (cos^2(th) - 1
  float tp = 1. / (dp*dp - 1.);
  return d / (.5 + .5 * sqrt(1. + sq(roughness) * sq(tp)));
}

float g2Naiive(vec3 l, vec3 v, vec3 h, float roughness){
  return g1Smith(h,l, roughness) * g1Smith(h, v, roughness);
}

// Eq 9.31 from Realtime Rendering
float g2smith(vec3 l, vec3 v, vec3 h, float roughness){
  float lh = dot(l,h), vh = dot(v,h);
  if(lh < 0. || vh < 0.) return 0.;

  float alpha = sq(roughness);
  float tanLH = 1. / (sq(lh) - 1.), tanVH = 1. / (sq(vh) - 1.);
  float lambdaL = sqrt(1. + alpha * sq(tanLH)), lambdaV = sqrt(1. + alpha * sq(tanVH));

  return 1. / (.5*lambdaL + .5*lambdaV);
}

float ggx(vec3 n, vec3 h, float roughness){
  float alpha = sq(roughness);
  float d = dot(n,h);
  float dp = step(0., d);
  float denom = PI * sq(1. + d*d * (alpha-1.));
  return d * alpha / denom;
}

vec3 schlick(vec3 h, vec3 l, vec3 f0){
  float d = max(0., dot(h,l));
  return f0 + (1. - f0) * pow(1. - d, 5.);
}

vec3 BRDF(vec3 v, vec3 l, vec3 n, float metalness, float roughness, vec3 rho){

  vec3 h = normalize(v + l);

  vec3 f0 = mix(vec3(0.02), rho, metalness);
  vec3 spec = schlick(h, l, f0);

  float mask = 1.;
  mask = g2smith(l,v,h,roughness);
  mask = g2Naiive(l,v,h,roughness);

  float ndf = ggx(n, h, roughness);

  vec3 fSpec = spec * mask * ndf;
  float kd = mix(1. - spec.r, 0., metalness);
  rho *= kd;

  vec3 diffuse = diffuse(rho, f0, n,  l,  v);
  float nlPlus = max(0., dot(n,l));
  vec3 outc = fSpec * nlPlus + diffuse * nlPlus;

  return clamp(outc, vec3(0.), vec3(1.));
}


void main(){
    vec3 c = vec3(1.);
    float metallic = texture(texture_specular1, fs_in.uv).r;
    float roughness = clamp(texture(texture_roughness1, fs_in.uv).r, 0.01, .99);
    vec3 albedo = texture(texture_diffuse1, fs_in.uv).rgb;
    vec3 nMap = texture(texture_normal1, fs_in.uv).rgb; // normal map is broke for the guitar model
    vec3 normal = fs_in.TBN * vec3(0., 0., 1.);

    metallic = smoothstep(0.6, 0.65, metallic);
    roughness = pow(roughness, 1.);
    vec3 l = -normalize(vec3(-2., -5., 2.));

    vec3 rd = normalize(fs_in.vPos - camPos);

    c = BRDF(-rd, l, normal, metallic, roughness, albedo);

    c += albedo * .1;

    FragColor = vec4(c, 1.);
}
