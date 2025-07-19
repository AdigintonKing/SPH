#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform float radius;
uniform float imgWidth, imgHeight;

in vec4 vPosition[];

out vec2 vTexCoord;

void main(void){

    // For each vertex moved to the right position on the vertex shader
    // it makes 6 more vertex that makes 2 GL_TRIANGLE_STRIP
    // that´s going to be the frame for the pixels of the sparkImg texture
    //
    for(int i = 0; i < gl_in.length(); i++){
        gl_Position = projection * view * model * ( vPosition[i] + vec4(-radius,-radius,0.0,0.0));
        vTexCoord.x = 0.0;
        vTexCoord.y = 0.0;
        EmitVertex();
        
        gl_Position = projection * view * model * (vPosition[i] + vec4(radius,-radius,0.0,0.0));
        vTexCoord.x = imgWidth;
        vTexCoord.y = 0.0;
        EmitVertex();
        
        gl_Position = projection * view * model * (vPosition[i] + vec4(radius,radius,0.0,0.0));
        vTexCoord.x = imgWidth;
        vTexCoord.y = imgHeight;
        EmitVertex();
        EndPrimitive();

        gl_Position = projection * view * model * (vPosition[i] + vec4(-radius,-radius,0.0,0.0));
        vTexCoord.x = 0.0;
        vTexCoord.y = 0.0;
        EmitVertex();
        
        gl_Position = projection * view * model * (vPosition[i] + vec4(-radius,radius,0.0,0.0));
        vTexCoord.x = 0.0;
        vTexCoord.y = imgHeight;
        EmitVertex();
        
        gl_Position = projection * view * model * (vPosition[i] + vec4(size,size,0.0,0.0));
        vTexCoord.x = imgWidth;
        vTexCoord.y = imgHeight;
        EmitVertex();
        EndPrimitive();
    }
}
