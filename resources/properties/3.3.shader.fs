#version 330 core
out vec4 FragColor;

in vec4 ourColor;
in vec3 Normal;
in vec3 FragPos;
in vec3 I;
uniform vec3 lightPos;
uniform vec3 lightPos1;
uniform vec3 lightColor;
uniform vec4 objectColor;
uniform vec3 viewPos;
uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;




float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}
void main()
{
    //ambient
    float ambientStrength = 0.33;
    float intensity =0.35;
    int edgefalloff = 16;
    float diffuseStrength=0.5;
    vec3 ambient = ambientStrength * lightColor;

// diffuse
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(lightPos - FragPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 lightDir1 = normalize(lightPos1 - FragPos);
float diff1 = max(dot(norm, lightDir1), 0.0);
vec3 diffuse = diffuseStrength*(diff+diff1) * lightColor;

// specular
float specularStrength = 1.0;
vec3 viewDir = normalize(viewPos - FragPos);
vec3 reflectDir = reflect(-lightDir, norm);
vec3 reflectDir1 = reflect(-lightDir1, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), 16);
vec3 specular = specularStrength * (spec+spec1) * lightColor;

float opac = dot(norm,viewDir);
opac = abs(opac);
opac = ambientStrength + intensity*(1.0-pow(opac,edgefalloff));

// calculate shadow
//float shadow = ShadowCalculation(FragPosLightSpace);
vec4 result = vec4(ambient+ (1.0 ) *diffuse+specular,1.0) *objectColor;
result*=opac;
result.a*=opac;
FragColor = result;
}
