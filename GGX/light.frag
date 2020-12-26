#define ENTRY
precision highp float;

uniform vec2 resolution;
uniform float time;
const float PI = 3.14159265;

mat2 rot(float t){
  float c = cos(t), s = sin(t);
  return mat2(c,s,-s,c);
}

float smoothmin(float a, float b, float k){
  float h = clamp( 0.5 + 0.5*(b-a)/k, 0., 1.);
  return mix( b, a, h ) - k*h*(1.0-h);
}

float map(vec3 q){
  float d = 100.;
  vec3 p = q;
  p.y -= 1.4;
  p.x += 3.;
  float sp = length(p) - 1.;
  d = min(d, sp);

  p = q;
  p.x -= 3.;
  // p.xz *= rot(time);
  // p.yz *= rot(time);
  vec3 b = abs(p) - vec3(.7);
  float bx = smoothmin(b.x, smoothmin(b.y, b.z, -.2), -.2);
  d = min(d, bx);

  p = q;
  p.yz *= rot(PI/2.);
  float x = length(p.xz) - 1.;
  float y = p.y;
  vec2 tp = vec2(x,y);
  float tr = length(tp)-.3;
  d = min(d, tr);



  p = q;
  d = smoothmin(d, p.y, .4);
  // d = min(d, p.y);

  return d;
}

vec3 normal(vec3 p){
  const float eps = 0.000287;
  const vec2 h = vec2(eps,0);
  return normalize( vec3(map(p+h.xyy) - map(p-h.xyy),
                         map(p+h.yxy) - map(p-h.yxy),
                         map(p+h.yyx) - map(p-h.yyx) ) );
}

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
  return fSpec *  nlPlus + diffuse * nlPlus;
}


float calcSoftshadow(vec3 ro, vec3 rd, float mint, float tmax)
{
  float res = 1.;
  float t = mint;
  float ph = 1e10; // big, such that y = 0 on the first iteration

  for( int i=0; i<32; i++ )
  {
    float h = map( ro + rd*t );
    // use this if you are getting artifact on the first iteration, or unroll the
    // first iteration out of the loop
    float y = (i==0) ? 0.0 : h*h/(2.0*ph);
    // float y = h*h/(2.0*ph);
    float d = sqrt(h*h-y*y);
    res = min( res, 10.0*d/max(0.0,t-y) );
    ph = h;

    t += h;
    if( res<0.0001 || t>tmax ) break;
  }
  return clamp( res, 0.0, 1.0 );
}

void main(){

  vec2 uv = gl_FragCoord.xy / resolution.xy;
  vec3 c = vec3(0.);
  vec2 pt = uv*2. - 1.;
  pt.y *= resolution.y / resolution.x;

  float tm = time * .3;
  vec3 ro=vec3(0.,0.,0.);
  ro += vec3(0., 5., 0.);
  ro += vec3(cos(tm), 0., sin(tm)) * 5.;
  // ro += vec3(cos(PI/4.), 0., sin(-PI/4.)) * 6.;

  vec3 up = vec3(0.,1.,0.);
  vec3 focus = vec3(0., 0., 0.);
  vec3 rov = normalize(focus - ro),
  u = normalize(cross(rov, up)),
  v = -cross(rov,u);
  vec3 rd = mat3(u, v, rov) * normalize(vec3(pt, 1.));

  vec3 p = ro;
  float t=0., d=0.;
  for(int i=0; i < 100; ++i){
    p = ro + rd*t;
    d = map(p);

    t += d;

    if(d < 0.01 || d > 120.) break;
  }


  if(d < 0.01) {
    vec3 n = normal(p);
    float nv = max(0., dot(-rd, n));
    vec3 rho = vec3(.9);

    // sphere light from ch 10.
    // only works on diffuse lambert
    vec3 pl = vec3(-3., 4., -4.);
    float radius = 3.;
    vec3 L = vec3(.3, .9, .9);
    vec3 e = pl - p;
    vec3 l = normalize(e);
    vec3 h = normalize(l - rd);
    float nl = max(0., dot(n,l)),
    hn = max(0., dot(h, n));

    vec3 cl = L * rho * sq(radius) / sq(length(pl-p));
    float shadow = calcSoftshadow(p, l, .1, 20.);
    if(dot(n,l) < 0.) shadow = 0.;
    c += nl * cl * clamp(.4 + shadow, 0., 1.);

    vec3 pl2 = vec3(3., 4., 4.);
    vec3 e2 = pl2 - p;
    float radius2 = 4.;
    vec3 L2 = vec3(.9, 0.3, .9);
    vec3 l2 = normalize(e2);
    vec3 h2 = normalize(l2 - rd);
    float nl2 = max(0., dot(n,l2)),
    hn2 = max(0., dot(h2, n));

    vec3 cl2 = L2 * rho * sq(radius2) / sq(length(e2));
    float shadow2 = calcSoftshadow(p, l2, .1, 20.);
    if(dot(n,l2) < 0.) shadow2 = 0.;
    c += nl2 * cl2 * clamp(.4 + shadow2, 0., 1.);

    c += rho * .2;
  }


  c = smoothstep(0., 1., c);
  c = pow(c, vec3(.4545));



  gl_FragColor = vec4(c, 1.0);
}
