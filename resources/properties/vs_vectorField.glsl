#version 330

uniform mat4 modelview_matrix;
uniform mat4 projection_matrix;

//uniform float radius;
uniform float viewport_width;
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color_field;
layout (location = 2) in vec3 velocity_field;
//layout (location = 3) in vec3 acceleration_field;

out VS_OUT {
        flat vec4 color_field;
        flat vec3 vel_field;
}
vs_out;

void main()
{
    vec4 mv_pos = modelview_matrix * vec4(position,1.0);
    //vec4 proj = projection_matrix * vec4(radius, 0.0, mv_pos.z, mv_pos.w);
    //gl_PointSize = viewport_width * proj.x / proj.w;
     mat3 vecMatrix = mat3(modelview_matrix);

        vs_out.color_field = color_field;
        vs_out.vel_field = vec3(vec4(vecMatrix*velocity_field,0.0));
        gl_Position = projection_matrix * mv_pos;
}
