precision highp float;

uniform vec2 resolution;
uniform float time;
uniform sampler2D backbuffer;
uniform sampler2D tNoise;

const float PI = 3.14159265;
const float TAU = 2. * PI;


float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

vec3 chmin(vec3 d, vec3 obj){
  return d.x<obj.x ? d : obj;
}
vec3 map(vec3 q){
  vec3 p = q;
  vec3 d = vec3(10000., 0,0);
  vec2 uv = fract(p.xz / 20.);
  p += texture2D(tNoise, uv).r;
  vec3 pl = vec3(p.y + 1.5, 0,0);
  d = chmin(d,pl);

  return d;
}

mat2 r2d(float th){
  float c = cos(th), s = sin(th);
  return mat2(c,s,-s,c);
}

float mapVolume(vec3 p){
  float d = 0.;

  p.xy *= r2d(-PI * .2);
  float x = length(p.xz) - 3.;
  float y = p.y;
  float torus = length(vec2(x,y)) - 1. - noise(p*3.)*.2;

  d += noise(p*4.) * smoothstep(0., 1., -torus);

  return max(0., d);
}

vec3 calcNormal(vec3 p){
  vec2 d = vec2(.000387,0);
  return normalize(vec3(
    map(p + d.xyy).x - map(p - d.xyy).x,
    map(p + d.yxy).x - map(p - d.yxy).x,
    map(p + d.yyx).x - map(p - d.yyx).x
    ));
}

const vec3 up = vec3(0,1.,0);

float HenyeyGreenstein(float cosTheta, float g){
  float gsq = g*g;
  return (1. - gsq) / (4. * PI * pow(1. +  gsq  - 2.*g*cosTheta, 1.5));
}

void main(){
  vec2 uv = gl_FragCoord.xy / resolution;
  vec2 pt = (uv - .5) * 2.;
  pt.y *= resolution.y / resolution.x;

  vec3 c = vec3(0);

  vec3 ro = vec3(0);
  // ro += vec3(0,2.,5.);
  ro += vec3(0,1.6,0);
  float tm = mod(time, 1000.);
  ro += vec3(cos(tm), 0., sin(tm)) * 7.;
  vec3 focus = vec3(0);
  vec3 rov = normalize(focus - ro);
  vec3 cu = normalize(cross(rov, up));
  vec3 cv = cross(cu, rov);
  vec3 rd = normalize(mat3(cu,cv, rov) * vec3(pt,1.));

  vec3 p = ro;
  float t=0.;
  vec3 d = vec3(0);

  for(int i = 0; i < 128; i++){
    p = ro + rd*t;
    d = map(p);
    t += d.x;
  }

  vec3 lpos = vec3(6., 7., 0.);
  vec3 lc = vec3(1.);

  if(d.x < 0.01){
    vec3 n = calcNormal(p);
    vec3 l = normalize(lpos-p);
    float r = length(lpos - p);
    float intensity = 50. / (r*r);
    vec3 albedo = vec3(.5);
    c += intensity * lc * albedo * max(0.1, dot(n,l));
  }

  float tVol = 0.;
  float st = .25;
  vec3 volColor = vec3(0.);
  vec3 extinction = vec3(1., .6, .5)*10.;
  float opticalDepth = 0.;
  for(int i=0; i < 64; i++){
    p = ro + rd*tVol;
    bool isin = map(p).x < 0.;

    vec3 p2l = normalize(lpos - p);
    float r = length(lpos - p);
    float tScat = 0.;
    float rho = mapVolume(p);
    opticalDepth += rho*st;
    float rhoScat = 0.;
    float scatOpticalDepth = 0.;
    float scatSt = st * .2;
    for(int j=0; j < 10; j++){
      vec3 pp = p+p2l*tScat;
      rhoScat = mapVolume(p);
      scatOpticalDepth += rhoScat*scatSt / (r*r);
      tScat += scatSt;
    }
    float cosTheta = dot(p2l, rd);
    float p0 = HenyeyGreenstein(cosTheta, 0.4);
    float p1 = HenyeyGreenstein(cosTheta, 0.1);
    float phase = p0 + .99 * (p1 - p0);
    vec3 lscatin = PI * phase * exp(-scatOpticalDepth*extinction);
    vec3 transmission = exp(-opticalDepth*extinction);
    volColor += transmission * opticalDepth * lscatin / (r*r);
    tVol += st;
  }
  c *= .5;
  c += volColor * 25.;

  gl_FragColor = vec4(c,1.);
}
