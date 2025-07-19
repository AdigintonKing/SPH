#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec4 oColor;
in vec3 WorldPos;

uniform float radius;
uniform vec4 Color;
uniform sampler2D sprite;
uniform vec3 lightPosition;
uniform vec3 lightPosition1;
uniform vec3 camPos;

//uinform float sphereRadius;

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


void main()
{
// calculate normal
vec3 n;
n.xy = gl_PointCoord* 2.0 - vec2(radius);
float mag = dot(n.xy, n.xy);
if (mag > 1.0) discard;   // kill pixels outside circle
n.z = sqrt(1.0-mag);


// calculate lighting
vec3 lightColor = vec3(1.0,1.0,1.0);
// specular
//ambient
float ambientStrength = 0.15;
vec3 ambient = ambientStrength * lightColor;

// diffuse
float diffuseStrength = 0.6         ;
vec3 norm = normalize(n);
vec3 lightDir = normalize(lightPosition - WorldPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 lightDir1 = normalize(lightPosition - WorldPos);
float diff1 = max(dot(norm, lightDir1), 0.0);
vec3 diffuse = diffuseStrength*(diff+diff1) * lightColor;

// specular
float specularStrength = 0.7;
vec3 viewDir = normalize(camPos - WorldPos);
vec3 reflectDir = reflect(lightDir, norm);
vec3 reflectDir1 = reflect(lightDir1, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), 16);
vec3 specular = specularStrength * (spec+spec1) * lightColor;
vec4 result = vec4(ambient+diffuse+specular,1.0f) * oColor;

    FragColor = (texture(sprite, TexCoords) * result);
}
