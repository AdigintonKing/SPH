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

void main()
{
    //ambient
    float ambientStrength = 0.1;
    float intensity =0.8;
    int edgefalloff = 64;
    vec3 ambient = ambientStrength * lightColor;




// diffuse
float diffuseStrength = 0.6;
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(lightPos - FragPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 lightDir1 = normalize(lightPos1 - FragPos);
float diff1 = max(dot(norm, lightDir1), 0.0);
vec3 diffuse = diffuseStrength*(diff+diff1) * lightColor;




// specular
float specularStrength = 0.3;
vec3 viewDir = normalize(viewPos - FragPos);
vec3 reflectDir = reflect(lightDir, norm);
vec3 reflectDir1 = reflect(lightDir1, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), 8);
vec3 specular = specularStrength * (spec+spec1) * lightColor;
float opac = dot(norm,viewDir);
opac = abs(opac);
opac = ambientStrength + intensity*(1.0-0.5*pow(opac,edgefalloff));

vec4 result = vec4(ambient+diffuse+specular,1.0f) * ourColor;
result.a*=opac;
FragColor = opac*result;
}
