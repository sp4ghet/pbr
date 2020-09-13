/*{
"PASSES": [{
        TARGET: "accum",
        FLOAT: true,
    }, {
        TARGET: "view",
        FLOAT: true,
    }]
}*/

precision mediump float;

uniform vec2 resolution;
uniform float time;

uniform int	PASSINDEX;
uniform sampler2D accum;
uniform sampler2D view;
uniform sampler2D backbuffer;

#define MAX_FLOAT 120.
#define MAX_RECURSION 10
#define PI 3.1415926535897932385
#define TAU 2. * PI
// Φ = Golden Ratio
#define PHI 1.61803398874989484820459

vec2 seed;

vec2 rand2n() {
    seed+=vec2(-1,1);
	// implementation based on: lumina.sourceforge.net/Tutorials/Noise.html
    return vec2(fract(sin(dot(seed.xy ,vec2(12.9898,78.233))) * 43758.5453),
		fract(cos(dot(seed.xy ,vec2(4.898,7.23))) * 23421.631));
}

vec3 ortho(vec3 v) {
    //  See : http://lolengine.net/blog/2013/09/21/picking-orthogonal-vector-combing-coconuts
    return abs(v.x) > abs(v.z) ? vec3(-v.y, v.x, 0.0)  : vec3(0.0, -v.z, v.y);
}

vec3 getSampleBiased(vec3  dir, float power) {
	dir = normalize(dir);
	vec3 o1 = normalize(ortho(dir));
	vec3 o2 = normalize(cross(dir, o1));
	vec2 r = rand2n();
	r.x=r.x*2.*PI;
	r.y=pow(r.y,1.0/(power+1.0));
	float oneminus = sqrt(1.0-r.y*r.y);
	return cos(r.x)*oneminus*o1+sin(r.x)*oneminus*o2+r.y*dir;
}

vec3 getSample(vec3 dir) {
	return getSampleBiased(dir,0.0); // <- unbiased!
}

vec3 getCosineWeightedSample(vec3 dir) {
	return getSampleBiased(dir,1.0);
}

vec3 getConeSample(vec3 dir, float extent) {
        // Formula 34 in GI Compendium
	dir = normalize(dir);
	vec3 o1 = normalize(ortho(dir));
	vec3 o2 = normalize(cross(dir, o1));
	vec2 r =  rand2n();
	r.x=r.x*2.*PI;
	r.y=1.0-r.y*extent;
	float oneminus = sqrt(1.0-r.y*r.y);
	return cos(r.x)*oneminus*o1+sin(r.x)*oneminus*o2+r.y*dir;
}

// types

struct ray{
  vec3 o, dir;
};

struct hit{
  vec3 p,n;
  int m;
};

// ray primitive

vec3 ray_at(ray r, float t){
  return r.o + r.dir*t;
}

mat2 rot(float t){
  float c = cos(t), s=sin(t);
  return mat2(c,s,-s,c);
}
// raytracing
const vec2 t_min_max = vec2(0.001, 120.);

float mmap(vec3 q, inout int m){
  float d = 10000.;

  vec3 p = q;
  float size = 8.;
  float bx;
  vec3 bxs;
  float otherbx = d;
  for(int i=0; i < 3; i++){
    p = q;
    p[i] -= size;
    vec3 b = abs(p) - vec3(size / 1.9);
    bx = max(b.x, max(b.y,b.z));
    bxs[i] = bx;
    otherbx = min(otherbx, bx);
    d = min(bx, d);
  }
  for(int i=0; i < 2; i++){
    p = q;
    p[i] += size;
    vec3 b = abs(p) - vec3(size / 1.9);
    bx = max(b.x, max(b.y,b.z));
    d = min(bx, d);
    bxs[2] = i == 1 ? bxs[2] : bx;
    otherbx = min(bx, otherbx);
  }

  p = q;
  p.y += size / 2.1 - 1.;
  p.x -= 2.;
  vec3 b = abs(p) - vec3(1);
  float box2 = max(b.x, max(b.y, b.z));
  d = min(box2, d);

  p = q;
  p.y += size / 2.1 - 1.5;
  p.x += 1.5;
  p.xz *= rot(PI/4.);
  vec3 b2 = abs(p) - vec3(1,1.7,1);
  float box = max(b2.x, max(b2.y, b2.z));
  d = min(box, d);

  p = q;
  p.y -= size / 2.1;
  b = abs(p) - vec3(1., .1, 1.);
  float light = max(b.x, max(b.y, b.z));
  d = min(d, light);


  if(d == otherbx || d == box){
    m = 0;
  }
  if(d == light){
    m = 1;
  }
  if(d == box2){
    m = 2;
  }
  if(d == bxs[2]){
    m = 4;
  }
  if(d == bxs[0]){
    m = 3;
  }

  return d;
}

float map(vec3 q){
  int x=0;
  return mmap(q, x);
}


vec3 normal(vec3 p){
  const float eps = 0.002375;
  const vec2 h = vec2(eps,0);
  return normalize( vec3(map(p+h.xyy) - map(p-h.xyy),
                         map(p+h.yxy) - map(p-h.yxy),
                         map(p+h.yyx) - map(p-h.yyx) ) );
}

bool raycast(const in ray r, inout hit h){

  float t=0.01, d=0.;
  vec3 p = ray_at(r, t);
  for(int i=0; i < 100; i++){
    p = ray_at(r,t);
    d = mmap(p, h.m);
    if(d < t_min_max.x || d > t_min_max.y) break;

    t += clamp(d, 0.02, .9);
  }


  if(d < t_min_max.x){
    h.p = p;
    h.n = normal(p);
    return true;
  }
  return false;
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

struct mat {
  float roughness, metalness;
  vec3 albedo, emission;
};

mat getMaterial(int m){
  if(m == 0){
    return mat(.8, .01, vec3(0.5), vec3(0));
  }
  if(m == 1){
    return mat(.5, 0.01, vec3(0,0,0), vec3(4.));
  }
  if(m == 2){
    return  mat(.1, .9, vec3(.4, .8, .8), vec3(0));
  }
  if(m == 3){
    return mat(.8, .01, vec3(.3, .8, .3), vec3(0));
  }
  if(m == 4){
    return mat(.8, .01, vec3(.9, .3, .3), vec3(0));
  }
}

vec3 brdf(inout ray r, inout hit ht, in vec3 col){

  vec3 hemi = getCosineWeightedSample(ht.n);
  mat m = getMaterial(ht.m);
  vec3 rho = m.albedo;

  float metalness = m.metalness;
  float roughness = m.roughness;

  vec3 v = -r.dir;
  vec3 l = normalize(vec3(-4., 3., 1.));

  vec3 h = normalize(v + l);

  vec3 f0 = mix(vec3(0.02), rho, metalness);
  vec3 spec = schlick(h, l, f0);

  float mask = 0.;
  mask = g2smith(l,v,h,roughness);

  float ndf = ggx(ht.n, h, roughness);

  vec3 fSpec = spec * mask * ndf;
  vec3 diffuse = rho / PI;

  float rng = rand2n().r;
  float refl_prob = (spec.r + spec.g + spec.b) / 3.;
  if(rng < refl_prob){
    r.dir = reflect(r.dir, ht.n) + hemi * roughness;
    diffuse *= PI * roughness;
  }else{
    r.dir = hemi;
    diffuse *= PI;
  }
  r.o = ht.p + ht.n*.1;

  // PDF(OMEGA) = cos(theta)/PI;
  // therefore, taking the rendering equation:
  // f(l,v)L_i(l,v) dot+(n,l) dw
  // we need to modulate by 1/PDF -> L_i * f(l,v) * PI
  return col * (diffuse + fSpec) + m.emission * PI;
}

vec3 ray_color(ray r){
  vec3 col = vec3(1.);
  hit h;
  vec3 direct = vec3(0);
  for(int i=0; i < MAX_RECURSION; i++){
    bool is_hit = raycast(r, h);
    if(is_hit){
      col = brdf(r, h, col);

      // Direct lighting
      // vec3 sunDirection = normalize(vec3(-4., 3., 1.));
      // vec3 sunSampleDir = getConeSample(sunDirection,1e-5);
      // float sunLight = dot(h.n, sunSampleDir);
      // ray nr = ray(r.o, sunSampleDir);
      // hit nh;
      // if (sunLight>0.0 && !raycast(nr, nh)) {
      //   direct += col*sunLight * .2;
      // }
    }else{
      // skybox
      col *= vec3(1);
      return direct + col;
    }
  }
  return col;
}

void main(){
  vec2 uv = gl_FragCoord.xy / resolution;
  vec2 p = uv*2. - 1.;
  p.y *= resolution.y / resolution.x;
  // new seed every frame
  seed = fract(uv * fract(time) * mod(time * .001, 10.));


  // anti aliasing
  vec2 jitter = (2. * rand2n()) - 1.;
  vec2 st = uv + jitter * 0.001;

  // multisampling in veda
  // we use multipass, which does one pass (0) where we accumulate to a buffer (accum)
  // the second pass (1) takes the average of the accumulated values
  if(PASSINDEX == 0){
    const vec3 up = vec3(0,1,0);

    vec3 ro = vec3(0);
    ro += up * 1.7;
    // ro += vec3(0,0,-1) * 10.;
    float angle = PI - PI / 15.;
    ro += vec3(sin(angle), 0., cos(angle)) * 10.;

    vec3 focus = vec3(0);
    vec3 rov = normalize(focus - ro),
    u = normalize(cross(up,rov)),
    v = cross(rov,u);
    vec3 rd = mat3(u,v,rov) * normalize(vec3(p, 1.));
    ray r = ray(ro, rd);

    vec3 col = ray_color(r);

    vec4 result = vec4(col,1.) + texture2D(accum, uv);
    if(result.a > 1e12){
      result = vec4(result.xyz/result.a, 1.);
    }
    gl_FragColor = result;
  }

  // write to view
  if(PASSINDEX == 1){
    vec4 result = texture2D(accum, uv);

    // gamma correction
    vec3 c = result.xyz / result.a;
    c = smoothstep(0., 1., c);
    // c = pow(c, vec3(.4545));

    gl_FragColor = vec4(c, 1.);
  }
}
