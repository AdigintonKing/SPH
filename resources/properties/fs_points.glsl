#version 330

uniform float radius;
uniform float min_scalar;
uniform float max_scalar;
uniform bool vok;
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

void main(void)
{
    // calculate normal 
    vec3 n;
    n.xy = gl_PointCoord* 2.0 - vec2(1.0);    
    float mag = dot(n.xy, n.xy);
    if (mag > 1.0) discard;   // kill pixels outside circle
    n.z = sqrt(1.0-mag);

    // calculate lighting
	const vec3 light_dir = vec3(0.0, 0.0, 1.0);
    float diffuse = max(0.0, dot(light_dir, n));
 
        vec3 eye = In.mv_pos + vec3(0.0, 0.0, radius * n.z);
    vec3 halfVector = normalize( eye + light_dir);	
    float spec = pow(max(0.0, dot(n,halfVector)), 100.0);
	
	float depth = (projection_matrix[2][2] * eye.z + projection_matrix[3][2])
        / (projection_matrix[2][3] * eye.z + projection_matrix[3][3]);

    gl_FragDepth = (depth + 1.0) / 2.0;
      vec3 fluidColor;

        if(vok)
        {
            float nv = norm(In.vel_field);
            float k =(nv-min_scalar)/(max_scalar-min_scalar);
            nv =min(k, 1.0);
            nv = min(0.75*nv, 0.75);
            fluidColor =hsv2rgb(vec3(0.75- nv, 1.0f, 1.0f));
        }
        else
        {
        // modify color according to the scalar field
        //vec3 hsv = rgb2hsv(vec3(color));
        //float v = max(In.scalar_field-min_scalar, 0.0);
        //float diff = abs(max_scalar-min_scalar);
        //v = min(v/diff, 1.0);
        fluidColor = In.color_field.xyz;
        //vec3(In.color_field);//hsv2rgb(vec3(hsv.x, 1.0 - v, 1.0));
        }
	// compute final color
        vec3 color_ = 0.2 * fluidColor;
        color_ += 0.8 * diffuse * fluidColor;
        color_ += 0.5 * spec * vec3(1.0);
	color_ = clamp(color_, 0.0, 1.0);
	
    out_color = vec4(color_, In.color_field.w);
}
