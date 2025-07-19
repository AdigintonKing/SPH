#version 330 core
// components per vertex
#define COMP_POS          0 // vertex position
#define COMP_VNX          1 // vector to next vertex
#define COMP_NNX          2 // normalized vector to next vertex
#define COMP_NRM          3 // normal
#define TEXELS_PER_VERTEX 4

#define TheTime (iTime * 1.0)

#define TEXEL_MAG 5


layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
}  
