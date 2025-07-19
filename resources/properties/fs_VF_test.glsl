#version 330

uniform float radius;
uniform float min_scalar;
uniform float max_scalar;
uniform bool vok;
uniform  bool imageok;
uniform mat4 projection_matrix;

in block
{
	flat vec3 mv_pos;
        flat vec4 color_field;
        flat vec3 vel_field;
}
In;

out vec4 out_color;
uniform vec4 color;



const float PI = 3.1415927;



const int   ARROW_V_STYLE = 1;
const int   ARROW_LINE_STYLE = 1;

// Choose your arrow head style
const int   ARROW_STYLE = ARROW_LINE_STYLE;
const float ARROW_TILE_SIZE = 40.0;

// How sharp should the arrow head be? Used
const float ARROW_HEAD_ANGLE = 45.0 * PI / 180.0;

// Used for ARROW_LINE_STYLE
const float ARROW_HEAD_LENGTH = ARROW_TILE_SIZE / 6.0;
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
                mag_v = clamp(mag_v, 5.0, ARROW_TILE_SIZE / 2.0);

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

// The vector field; use your own function or texture
vec2 field(vec2 pos) {
            return vec2(cos(pos.x * 0.01 + pos.y * 0.01) + cos(pos.y * 0.005 ), 2.0 * cos(pos.y * 0.01  ));

        // Examples:
//	return 2.0 * texture(iChannel1, mod(pos, 2.0 * iChannelResolution[1].xy) * 0.5 / iChannelResolution[1].xy).xy - 1.0;
//	return 2.0 * texture(iChannel0, (pos + vec2(iTime * 100.0, 0.0)) / iChannelResolution[0].xy).xy - 1.0;
//	return vec2(0.0, 0.0);
//	return vec2(cos(pos.x * 0.017 + cos(pos.y * 0.004 + iTime * 0.1) * 6.28 * 4.0) * 3.0, cos(6.28 * cos(pos.y * 0.01 + pos.x * 0.007)));
}

void main(void)
{
    // calculate normal 
    vec3 n;
    n.xy = gl_PointCoord* 2.0 - vec2(1.0);
    float mag = dot(n.xy, n.xy);
    if (mag > 1.0) discard;   // kill pixels outside circle
    n.z = sqrt(1.0-mag);

    // calculate lighting
        const vec3 light_dir = vec3(0.2, 0.2, 1.0);
    float diffuse = max(0.0, dot(light_dir, n));
 
        vec3 eye = In.mv_pos + vec3(0.0, 0.0, 2.0*radius * n.z);
    vec3 halfVector = normalize( eye + light_dir);	
    float spec = pow(max(0.0, dot(n,halfVector)), 100.0);
	
	float depth = (projection_matrix[2][2] * eye.z + projection_matrix[3][2])
        / (projection_matrix[2][3] * eye.z + projection_matrix[3][3]);

    gl_FragDepth = (depth + 1.0) / 2.0;
      vec3 fluidColor;
       vec3 color_;
        if(vok)
        {
            float nv = norm(In.vel_field);
            float k =(nv-min_scalar)/(max_scalar-min_scalar);
            nv =min(k, 1.0);
            nv = min(0.72*nv, 0.72);
            fluidColor =hsv2rgb(vec3(0.72- nv, 1.0f, 1.0f));
            color_ = 0.2 * fluidColor;
            color_ += 0.8 * diffuse * fluidColor;
            color_ += 0.5 * spec * vec3(1.0);
            color_ = clamp(color_, 0.0, 1.0);
        }
        else
        {
            // modify color according to the scalar field
            vec3 hsv = rgb2hsv(vec3(color));
            float magv= length(In.vel_field);
            float v = max(magv-min_scalar, 0.0);
            float diff = abs(max_scalar-min_scalar);
            v = min(v/diff, 1.0);
            fluidColor = In.color_field.xyz;
            //vec3(In.color_field);
            fluidColor[1] = min(0.8,0.8-v);//hsv2rgb(fluidColor);
            fluidColor[2]*=0.9;

        // compute final color
        color_ = 0.2 * fluidColor;
        color_ += 0.8 * diffuse * fluidColor;
        color_ += 0.5 * spec * vec3(1.0);
        color_ = clamp(color_, 0.0, 1.0);
        }
        if(imageok)
        {
        fluidColor = In.color_field.xyz;
        color_ = 0.9* fluidColor;
        //color_ += 0.5 * spec * vec3(1.0);
        //color_ = clamp(color_, 0.0, 1.0);
        }

        //vec2 tmp_field = clamp(In.vel_field.xy,0.0,1.0);
    //out_color = vec4(color_, In.color_field.w);
        if(vok)
        {
            float Arrowfactor = (1-arrow(gl_FragCoord.xy, In.vel_field.xy * ARROW_TILE_SIZE *0.2));
            if(Arrowfactor<0.15)
                out_color = vec4(vec3(1.0),In.color_field.w);
            else
            out_color =  Arrowfactor*vec4(color_, In.color_field.w);
        }
            else
            out_color = vec4(color_, In.color_field.w);
}
