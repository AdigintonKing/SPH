#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aOffset;
layout (location = 3) in vec4 aColor;

out vec2 TexCoords;
out vec4 oColor;
out vec3 WorldPos;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;


void main()
{
    oColor = aColor;
    TexCoords = aTexCoords;
    WorldPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos + aOffset, 1.0);
}
