// see "Scalable Real-Time Animation of Rivers" https://hal.inria.fr/inria-00345903/

// noise-based variant: https://www.shadertoy.com/view/Xl3Gzj
#version 330
out vec4 FragColor;
//  uniform float radius;
uniform float min_scalar;
uniform float max_scalar;
uniform bool vok;
uniform mat4 projection_matrix;
uniform float radius;
uniform vec2 iResolution;

in vec4 fColor;
in vec3 f_vel;


uniform vec4 color;



#define r 1.0 // test distance      for efficiency, as small as it keep covering influencials river sides
#define c 1.5 // boundary condition   < 1: slip condition   > 1 : no-slip condition

#define L(a,b) FragColor+= 1e-3/length( clamp( dot(U-(a),v=b-(a))/dot(v,v), 0.,1.) *v - U+a ) // segments
    
float f(float d) { // interpolation function
    float t = 1.-d/r;
    return t<0. ? 0. :pow(d,-c)*(6.*t*t-15.*t+10.)*t*t*t;
}

void main(void)
{
    vec2 U = gl_FragCoord.xy;
    // calculate normal
    vec3 n;
    n.xy = gl_PointCoord* 2.0 - vec2(1.0);
    n.z = 1.0;

    // calculate lighting
        const vec3 light_dir = vec3(0.0, 0.0, 1.0);
     float diffuse = max(0.0, dot(light_dir, n));
    vec3 eye = vec3(gl_PointCoord,0.0) + vec3(0.0, 0.0, radius * n.z);
  vec3 halfVector = normalize( eye + light_dir);
float spec = pow(max(0.0, dot(n,halfVector)), 100.0);
vec3 fluidColor = vec3(fColor);
vec3 color_ = 0.2 * fluidColor;
color_ += 0.8 * diffuse * fluidColor;
color_ += 0.1 * spec * vec3(1.0);
color_ = clamp(color_, 0.0, 1.0);
        U /= iResolution.xy;
    
    // --- interpolate stream function : add(distance to border, stream at border)
    float wf=0.,wT=0., w;
#define add(d,phi)  w = f(d),  wf += w*phi,  wT += w;
    // river bed and obstacle geometry, + flux (diff of stream between river sides)
    /*add( length( U-vec2(.0+.2*sin(t) , .8)    ) -.3, 0.);   // rock1 stream=0.
    add( length( U-vec2(.3+.2*sin(t) , .5)    ) -.3, 0.);
    add( length( U-vec2(1.3, .8+.2*cos(2.*t)) ) -.5, .5);   // rock2 stream=.5
    add( length( U-vec2(.8+.2*cos(.5*t), -.2) ) -.4, 1.);   // rock3 stream=1.
    add( length( U-vec2(1.7, .15)             ) -.1,  .75); // rock4 stream=.75
    w = wf / wT;                               // stream field
    vec2 V = vec2(-dFdy(w), dFdx(w));          // velocity field
   */
    add(length(f_vel.xy),0.0);
    w = wf / wT;
    vec2 V = normalize(f_vel.xy);          // velocity field
    // --- display
    if (w!=w) { FragColor =  vec4(0.6745, 0.0, 0.0, 0.0); return; } // in rocks   ( w!=w = NaN )
        //FragColor = vec4(0,length(f_vel.xy),sin(100.*w),fColor[3]);   // draw |V| and iso-streams
    FragColor = vec4(0.0,length(f_vel.xy)/max_scalar,sin(200.*w),fColor[3]);   // draw |V| and iso-streams

    vec2 p = floor(U*30.+1.0)/30., v;           // draw velocity vectors
    L ( p-V*2., p+V*2.);                               // L(vec2(.5,.5),p);
}
