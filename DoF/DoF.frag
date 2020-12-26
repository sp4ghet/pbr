/*{
"PASSES": [{
        TARGET: "render",
        FLOAT: true,
    }, {
        TARGET: "vertical",
        FLOAT: true,
    },
    {
      TARGET: "diagonal",
      FLOAT: true
    },
    {
    TARGET: "render",
     FLOAT: true
    }
    ]
}*/

precision highp float;

uniform vec2 resolution;
uniform float time;
uniform sampler2D tNoise;

uniform int	PASSINDEX;
uniform sampler2D render;
uniform sampler2D vertical;
uniform sampler2D diagonal;

mat2 rotate(float a){
  float c = cos(a), s=sin(a);
  return mat2(c,s,-s,c);
}

vec3 smin(vec3 p1, vec3 p2, float k){
    vec3 h = clamp( 0.5 + 0.5*(p2-p1)/k, vec3(0.), vec3(1.));
    return mix( p2, p1, h ) - k*h*(1.0-h);
}
float smin( float d1, float d2, float k ) {
    float h = clamp( .5 + .5*(d2-d1)/k, 0., 1.);
    return mix( d2, d1, h) - k*h*(1.-h);
}

vec3 fractal(vec3 p, float t){
  float s = .3;

  for(int i=0; i < 4; i++){
    float tt = t + float(i);
    p.xz *= rotate(tt);
    p.xy *= rotate(tt * 1.378);

    p = smin(p, -p, -1.);
    p -= s;
    s *= 2.3;
  }

  return p;
}


float map(vec3 p){
  float d = 10000.;

  float noise =0.;
  float tmod = .1*0.016;
  noise = texture2D(tNoise, fract(p.xz * .001 + time*tmod)).r;
  noise = texture2D(tNoise, abs(fract(p.xz * .003 + time*tmod + noise))).r * 5.;
  noise += sin(length(p.xz)*.1 - time*.4) * 2.;

  vec3 pp = p;
  pp = fractal(pp, time * .4);

  float sp = length(pp) - 2.;
  d = min(d, sp);



  float pl = -p.y + noise;
  d = smin(d, pl, 10.);

  float sp2 = length(p - vec3(32., -5., 32.)) - .5;
  d = min(sp2, d);

  return d;
}

vec3 normal(vec3 p){
  const float eps = 0.00001;
  const vec2 h = vec2(eps,0);
  return normalize( vec3(map(p+h.xyy) - map(p-h.xyy),
                         map(p+h.yxy) - map(p-h.yxy),
                         map(p+h.yyx) - map(p-h.yyx) ) );
}

vec4 BlurTexture(sampler2D tex, vec2 uv, vec2 direction)
{
    vec4 finalColor = vec4(0);
    float blurAmount = 0.0;

    // This offset is important. Will explain later. ;)
    uv += direction * 0.5;

    for (int i = 0; i < 40; ++i)
    {
        vec4 color = texture2D(tex, uv + direction * float(i));
        color *= color.a;
        blurAmount += color.a;
        finalColor += color;
    }

    return (finalColor / blurAmount);
}

const float PI = 3.14159265;

vec4 postProcess(vec2 uv){
  vec4 c;
  vec2 invRes = vec2(.35) / resolution;
  vec4 back = texture2D(render, uv);

  //vertical
  if(PASSINDEX == 1){
    float up = PI/2.;
    c += BlurTexture(render, uv, back.a * invRes * vec2(cos(up), sin(up)));
    c *= back.a;
    c.a = back.a;
  }
  // diagonal
  if(PASSINDEX == 2){
    float ur = -PI/6.;
    c += BlurTexture(render, uv, back.a * invRes * vec2(cos(ur), sin(ur)));
    c *= back.a;
    c.a = back.a;

  }
  //rhomb
  if(PASSINDEX == 3){
    float cocV = texture2D(vertical, uv).a;
    float cocD = texture2D(diagonal, uv).a;
    float dr = -PI/6.;
    float dl = -5. * PI / 6.;
    c += BlurTexture(vertical, uv, cocV * invRes * vec2(cos(dr), sin(dr))) * cocV;
    c += BlurTexture(vertical, uv, cocV * invRes * vec2(cos(dl), sin(dl))) * cocV;
    c += BlurTexture(diagonal, uv, cocD * invRes * vec2(cos(dl), sin(dl))) * cocD;
    c /= 3.;
  }

  return c;
}



void main(){

  vec2 uv = gl_FragCoord.xy / resolution.xy;
  vec2 pt = uv * 2. - 1.;
  pt.y *= resolution.y / resolution.x;

  if(PASSINDEX >= 1){
    gl_FragColor = postProcess(uv);
    if(PASSINDEX == 3){
      gl_FragColor.a = 1.;
    }
    return;
  }


  vec3 s=vec3(0.,0.,0.);
  float tm = PI;
  s.x += cos(tm) * 20.;
  s.z += sin(tm) * 20.;
  s.y -= 10.;
  vec3 up = vec3(0.,1.,0.);
  vec3 cd = normalize(vec3(0., -5., 0.) - s),
  cu = normalize(cross(cd,up)),
  cv = cross(cd,cu),
  r = mat3(cu,cv,cd) * normalize(vec3(pt, 1.)),
  p = s;
  float d=0., t = 0.;
  int mat = 0;

  for(int i=0; i < 100; i++){
    p = s + r*t;
    d = map(p);
    t += d;
    if(d < 0.01 || d > 120.) break;
  }


  vec3 c = vec3(0.);
  float rad = length(vec3(30., -5., 30.) - p);
  float i = 50. / (rad*rad);
  vec3 l = normalize(vec3(30., -5., 30.) - p);
  vec3 n = normal(p);
  vec3 h = normalize(l - r);
  float fre = pow(1. - abs(dot(n,r)), 5.);

  #define ao(a) clamp(map(p+n*a)/a,0.,1.)

  float sss = 0.;
  float aov = 0.;
  for(int i=1; i < 10; i++){
    float dist = float(i) * .5;
    sss += clamp(map(p + h*dist)/dist, 0., 1.);
    aov += ao(dist);
  }
  const vec3 cHighlight =  normalize(vec3(.9, .5, .5)) * 1.3;
  c += sss * .15 * cHighlight;
  c += aov * .01;
  c += max(0., dot(n,l)) * i * cHighlight;
  // c += fre * .3 *  (1. - cHighlight);

  float depth = t / 20. - 1.;
  depth = clamp(depth, -2., 2.);
  depth = abs(depth);

  gl_FragColor = vec4(c, depth);
}
