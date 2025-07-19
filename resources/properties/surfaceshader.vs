#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNormal;

out vec4 ourColor;
out vec3 Normal;
out vec3 FragPos;
out vec3 I;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 transform;
uniform float dir;
void main()
{
  vec4 P =view  *model* vec4(aPos, 1.0f);
  gl_Position = projection * P;
  I = P.xyz - vec3(0);
    ourColor = aColor;
    FragPos = vec3(model * vec4(aPos, 1.0));
   // Normal = aNormal;
   Normal = dir*mat3(transpose(inverse(model))) * aNormal;
}
