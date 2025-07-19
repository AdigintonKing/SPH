#version 330 core
in vec2 TexCoords;

out vec4 color;

uniform sampler2D sprite;
uniform vec4 Color;
// material parameters
uniform float metallic;
uniform float roughness;
uniform float ao;

// lights
uniform vec3 lightPosition;
uniform vec3 lightPosition1;
uniform vec3 lightColor;

uniform vec3 camPos;

void main()
{
    color = (texture(sprite, TexCoords) * Color);
}
