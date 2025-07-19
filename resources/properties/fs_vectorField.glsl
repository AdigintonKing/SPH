#version 330
out vec4 FragColor;
//  uniform float radius;
uniform float min_scalar;
uniform float max_scalar;
uniform bool vok;
uniform mat4 projection_matrix;
uniform float radius;

in vec4 fColor;
in vec3 f_vel;


uniform vec4 color;


float norm(vec3 v)
{
    return sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

const float PI = 3.1415927;



const int   ARROW_V_STYLE = 1;
const int   ARROW_LINE_STYLE = 1;

// Choose your arrow head style
const int   ARROW_STYLE = ARROW_LINE_STYLE;
const float ARROW_TILE_SIZE = 25.0;

// How sharp should the arrow head be? Used
const float ARROW_HEAD_ANGLE = 45.0 * PI / 180.0;

// Used for ARROW_LINE_STYLE
const float ARROW_HEAD_LENGTH = ARROW_TILE_SIZE / 4.0;
const float ARROW_SHAFT_THICKNESS = 2.0;



// Computes the center pixel of the tile containing pixel pos
vec2 arrowTileCenterCoord(vec2 pos) {
        return (floor(pos / ARROW_TILE_SIZE) + 0.5) * ARROW_TILE_SIZE;
}




// v = field sampled at tileCenterCoord(p), scaled by the length
// desired in pixels for arrows
// Returns 1.0 where there is an arrow pixel.
float arrow(vec2 p, vec2 v) {
        // Make everything relative to the center, which may be fractional
        p -= arrowTileCenterCoord(p);

    float mag_v = length(v), mag_p = length(p);

        if (mag_v > 0.0) {
                // Non-zero velocity case
                vec2 dir_p = p / mag_p, dir_v = v / mag_v;

                // We can't draw arrows larger than the tile radius, so clamp magnitude.
                // Enforce a minimum length to help see direction
                mag_v = clamp(mag_v, 2.0, ARROW_TILE_SIZE / 2.0);

                // Arrow tip location
                v = dir_v * mag_v;

                // Define a 2D implicit surface so that the arrow is antialiased.
                // In each line, the left expression defines a shape and the right controls
                // how quickly it fades in or out.

                float dist;
                if (ARROW_STYLE == ARROW_LINE_STYLE) {
                        // Signed distance from a line segment based on https://www.shadertoy.com/view/ls2GWG by
                        // Matthias Reitinger, @mreitinger

                        // Line arrow style
                        dist =
                                max(
                                        // Shaft
                                        ARROW_SHAFT_THICKNESS / 4.0 -
                                                max(abs(dot(p, vec2(dir_v.y, -dir_v.x))), // Width
                                                    abs(dot(p, dir_v)) - mag_v + ARROW_HEAD_LENGTH / 2.0), // Length

                                 // Arrow head
                                         min(0.0, dot(v - p, dir_v) - cos(ARROW_HEAD_ANGLE / 2.0) * length(v - p)) * 2.0 + // Front sides
                                         min(0.0, dot(p, dir_v) + ARROW_HEAD_LENGTH - mag_v)); // Back
                } else {
                        // V arrow style
                        dist = min(0.0, mag_v - mag_p) * 2.0 + // length
                                   min(0.0, dot(normalize(v - p), dir_v) - cos(ARROW_HEAD_ANGLE / 2.0)) * 2.0 * length(v - p) + // head sides
                                   min(0.0, dot(p, dir_v) + 1.0) + // head back
                                   min(0.0, cos(ARROW_HEAD_ANGLE / 2.0) - dot(normalize(v * 0.33 - p), dir_v)) * mag_v * 0.8; // cutout
                }

                return clamp(1.0 + dist, 0.0, 1.0);
        } else {
                // Center of the pixel is always on the arrow
                return max(0.0, 1.2 - mag_p);
        }
}



// Computes the signed distance from a line segment
float line(vec2 p, vec2 p1, vec2 p2) {
        vec2 center = (p1 + p2) * 0.5;
        float len = length(p2 - p1);
        vec2 dir = (p2 - p1) / len;
        vec2 rel_p = p - center;
        float dist1 = abs(dot(rel_p, vec2(dir.y, -dir.x)));
        float dist2 = abs(dot(rel_p, dir)) - 0.5*len;
        return max(dist1, dist2);
}

// v = field sampled at arrowTileCenterCoord(p), scaled by the length
// desired in pixels for arrows
// Returns a signed distance from the arrow
float arrow2(vec2 p, vec2 v) {
        // Make everything relative to the center, which may be fractional
        p -= arrowTileCenterCoord(p);

        float mag_v = length(v), mag_p = length(p);

        if (mag_v > 0.0) {
                // Non-zero velocity case
                vec2 dir_v = v / mag_v;

                // We can't draw arrows larger than the tile radius, so clamp magnitude.
                // Enforce a minimum length to help see direction
                mag_v = clamp(mag_v, 5.0, ARROW_TILE_SIZE * 0.5);

                // Arrow tip location
                v = dir_v * mag_v;

                // Signed distance from shaft
                float shaft = line(p, v, -v);
                // Signed distance from head
                float head = min(line(p, v, 0.4*v + 0.2*vec2(-v.y, v.x)),
                                 line(p, v, 0.4*v + 0.2*vec2(v.y, -v.x)));

                return min(shaft, head);
        } else {
                // Signed distance from the center point
                return max(0.0, 1.2 - mag_p);
        }
}

//shifts value range from 0-1 to -1-1
vec2 makeM1to1(vec2 x) {
    return (x - 0.5) * 2.0;
}

//shifts value range from -1-1 to 0-1
vec2 make0to1(vec2 x) {
    return (1.0 + x) / 2.0;
}


void main(void)
{
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
vec3 fluidColor;
float nv=fColor[3];
if(vok)
{
    float t = norm(f_vel);
    float k =(t-min_scalar)/(max_scalar-min_scalar);
     nv=min(k, 1.0);
    nv = min(0.75*nv, 0.75);
    fluidColor =hsv2rgb(vec3(0.75- nv, 1.0f, 1.0f));
}
else
{
    fluidColor=vec3(fColor);
}
vec3 color_ = 0.2 * fluidColor;
color_ += 0.8 * diffuse * fluidColor;
color_ += 0.1 * spec * vec3(1.0);
color_ = clamp(color_, 0.0, 1.0);

float ArrowFactor = (1-arrow2(gl_FragCoord.xy, f_vel.xy * ARROW_TILE_SIZE *0.25));
if(ArrowFactor>0.15)
    FragColor = vec4(1.0);
else
    FragColor  =  ArrowFactor*vec4(color_,nv);

    //FragColor  = (1-arrow(gl_FragCoord.xy, f_vel.xy * ARROW_TILE_SIZE *0.25))*vec4(color_,nv );
}
