precision highp float;

uniform vec2 resolution;
uniform float time;
uniform sampler2D backbuffer;
uniform sampler2D tNoise;

const float PI = 3.14159265;
const float TAU = 2. * PI;

mat2 r2d(float th){
  float c = cos(th), s = sin(th);
  return mat2(c,s, -s,c);
}

float map(vec3 q){
  vec3 p = q;
  float d = 100000.;
  vec3 n = vec3(0,1,0);

  vec2 uv = fract((p.xz + 67.) / 250.);
  float noise = 0.;
  float mult = 1.;
  float amp = 4.;
  for(int i=0; i < 3; i++){
    noise += texture2D(tNoise, fract(uv * mult)).r * amp;
    mult *= 2.;
    amp *= .78;
  }
  p += noise;

  float pl = p.y;
  d = min(d,pl);

  return d;
}

vec3 normal(vec3 p){
  const float eps = 0.000387;
  vec3 dd = vec3(eps,0,0);

  return normalize(vec3(
    map(p + dd) - map(p - dd),
    map(p+dd.yxy) - map(p - dd.yxy),
    map(p+dd.yyx) - map(p - dd.yyx)
  ));
}

float raySphere(vec3 ro, vec3 rd, vec3 center, float r){
  vec3 toC = ro - center;
  float b = 2. * dot(rd, toC);
  float c = dot(toC,toC) - r*r;
  float det = b*b - 4.*c;
  if(det < 0.) return -1.;
  float t = (-b + sqrt(det)) * .5;
  return t;
}
const float atmosRadius = 1000.;
const vec3 atmosCenter = vec3(0, -atmosRadius * .5, 0);

float density(vec3 p){
  float y = length(p - atmosCenter) * 2. / atmosRadius;
  float rho = exp(-y*5.);
  return rho * (1. - rho);
}

float opticalDepth(vec3 p, vec3 rd, float t){
  float ret = 0.;
  const int steps = 10;
  float stepsize = t / float(steps);
  float d = 0.;
  for(int j=0; j < steps; j++){
    d += stepsize;
    vec3 nowpos = p + rd*d;
    ret += density(nowpos) * stepsize;
  }
  return ret;
}

void main(){

  vec2 uv = gl_FragCoord.xy / resolution.xy;
  vec2 pt = (uv - .5) * 2.;
  pt.y *= resolution.y / resolution.x;
  vec3 c = vec3(0.);

  vec3 ro = vec3(0., 1.5, 0.);
  ro += vec3(0,0,5.);
  float tm = mod(time , 1000.) *  PI;
  // ro += vec3(cos(tm / 20.), 0., sin(tm / 20.)) * 10.;
  vec3 lookat = vec3(0., 1., 0.);
  const vec3 up = vec3(0,1,0);
  vec3 rov = normalize(lookat - ro);
  vec3 u = normalize(cross(rov, up));
  vec3 v = cross(u, rov);
  vec3 rd = mat3(u,v,rov) * normalize(vec3(pt, 1.));

  float t=0., d=0., thresh=.001;
  vec3 p = ro;
  bool hit = false;
  for(int i = 0; i < 128; i++){
    p = ro + rd * t;
    d = map(p);

    t += d * .9;

    hit = hit || d < thresh;
    if(hit) break;
  }

  vec3 l = normalize(vec3(sin(tm / 10.), cos(tm / 10.), 0.));
  l.xy *= r2d(PI / 4.);
  // vec3 l = normalize(vec3(0., 2., 1.));
  vec3 lpos = l * 1000.;

  if(hit){
    vec3 n = normal(p);
    c += max(0.05, dot(n,l)) * vec3(.8, .6, .3);
  }

  vec3 wavelengths = vec3(700., 550., 440.);
  vec3 extinction = .5 * pow(vec3(440.) / wavelengths, vec3(4.));

  vec3 pp = ro;
  float depth = hit ? t : raySphere(ro, rd, atmosCenter, atmosRadius);

  float stepsize = max(.01, depth / 32.);
  vec3 atmos = vec3(0);

  float viewRayOpticalDepth = 0.;
  for(int i=0; i<32; i++){
    depth -= stepsize;
    pp = ro + rd*depth;
    viewRayOpticalDepth += density(pp)*stepsize;
    vec3 p2l = normalize(lpos-pp);
    float t = raySphere(pp, p2l, atmosCenter, atmosRadius);
    float inscatDepth = opticalDepth(pp, p2l, t);
    vec3 tr = exp(-(viewRayOpticalDepth+inscatDepth*.5)*extinction);
    atmos += tr * density(pp) * stepsize * extinction;
  }

  c = mix(atmos, c, exp(-viewRayOpticalDepth));

  c = pow(c, vec3(.4545));

  gl_FragColor = vec4(c, 1.0);
}
