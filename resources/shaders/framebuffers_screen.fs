#version 330 core

// components per vertex
#define COMP_POS          0 // vertex position
#define COMP_VNX          1 // vector to next vertex
#define COMP_NNX          2 // normalized vector to next vertex
#define COMP_NRM          3 // normal
#define TEXELS_PER_VERTEX 1

#define TheTime (iTime * 1.0)

#define TEXEL_MAG 1
#define sqrt2 1.41421356237
#define refColor vec4(0.2, 0.1, 0.75, 1.0)
#define White vec4(1.0,1.0,1.0,1.0)

out vec4 Color;

in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform vec2 MaxvCoords;

//Shader Inputs
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iTime;                 // shader playback time (in seconds)
uniform float     iTimeDelta;            // render time (in seconds)
uniform int       iFrame;                // shader playback frame
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform sampler2D iChannel0;
uniform vec4      iDate;                 // (year, month, day, time in seconds)
uniform float     iSampleRate;           // sound sample rate (i.e., 44100)


#define LINEWIDTH 1.0

#define r 1.0 // test distance      for efficiency, as small as it keep covering influencials river sides
#define c 1.5 // boundary condition   < 1: slip condition   > 1 : no-slip condition

#define L(a,b) Color+= 5e-4/length( clamp( dot(U-(a),v=b-(a))/dot(v,v), 0.,1.) *v - U+a ) // segments


vec3 rgbToHsv(vec3 rgb){
    vec3 hsv = vec3(0);
    float maxC = max(max(rgb.x,rgb.g),rgb.b);
    float minC = min(min(rgb.x,rgb.g),rgb.b);
    float delta = maxC - minC;
    if(delta==0.0)
        return vec3(0);
    if (maxC == rgb.x) hsv.x = mod((rgb.g - rgb.b)/delta,6.0)/6.0;
    if (maxC == rgb.g) hsv.x = (rgb.b - rgb.x)/(delta*6.0) + 1.0/3.0;
    if (maxC == rgb.b) hsv.x = (rgb.x - rgb.g)/(delta*6.0) + 2.0/3.0;
    hsv.y = delta/maxC;
    hsv.z = maxC;
    return hsv;
}

float lineDist(vec2 p, vec2 a, vec2 v)
{
    vec2 pa = p - a;
    float h = clamp(dot(pa,v)/dot(v,v), 0.0, 1.0);
    return length(pa - v*h);
}





#define LOOKUP(COORD) texture(screenTexture,(COORD)/iResolution.xy)
vec4 Field (vec2 position) {
    // Rule 1 : All My Energy transates with my ordered Energy
    vec2 velocityGuess = LOOKUP (position).xy;
    vec2 positionGuess = position - velocityGuess;
        return LOOKUP (positionGuess);
}
const float offset = 1.0;

float f(float d) { // interpolation function
    float t = 1.-d/r;
    return t<0. ? 0. :pow(d,-c)*(6.*t*t-15.*t+10.)*t*t*t;
}

void main()
{

    vec2 Me= gl_FragCoord.xy;;
    vec4 C = LOOKUP(Me);
    vec2 center = MaxvCoords;
    vec3 Chsv =  rgbToHsv(C.rgb);

    float wf=0.,wT=0., w,ww;
    #define add(d,phi)  ww = f(d),  wf += ww*phi,  wT += ww;
    //add(0.75-Chsv.x,0.75);

    //w-=0.5;
    //

    vec2 U = Me/iResolution.xy;
    vec2 UU = Me/iResolution.y;
    float k = length(U-center)/sqrt2;
    add(k+0.01,1.0);
    //ww = wf / wT;
    w = (0.2  + (0.75-Chsv.x));
    //w=f(w);

    vec2 V = vec2(dFdy(ww), dFdx(ww))*0.05;
    V.x=mix(0.01*V.x,0.5*V.x, smoothstep(0.0,1.0,k*2.5));
    V.y =mix(0.01*V.y,0.5*V.y, smoothstep(0.0,1.0,k*2.5));
    if((C.x>0.9)&&(C.y>0.9)&&(C.z>0.9))
    Color = White;
    else{


    if ((U.x<0.01)||(U.x>0.99)||(U.y<0.04)||(U.y>0.99)) { Color =  vec4(1.0, 0.5, 0.2, 1.0); return; } // in rocks   ( w!=w = NaN )
    //Color = vec4(0,50*length(V),sin(100.*w),1.0);   // draw |V| and iso-streams
    Color = vec4(w,0.1,0.95-w,1.0);
    vec4 startColor;
    if((center.x==0.5)&&(center.y==0.5))
     startColor= C;
     else
     startColor=Color;


if(Chsv.x==0)
{
    Color = mix(startColor, refColor, smoothstep(0.0, 0.3,k));
    vec2 p = floor(U*25+.5)/25., v;           // draw velocity vectors
    //L(vec2(.5,.5),p);
    L( p-V*2., p+V*2.);
}

}

}
