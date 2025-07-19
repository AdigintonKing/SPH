

#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 2) out;


uniform float radius;
//uniform float imgWidth, imgHeight;

//in vec4 vPosition[];

out vec2 vTexCoord;

void main(void){
    float imgWidth=12;
    float imgHeight=12;
    // For each vertex moved to the right position on the vertex shader
    // it makes 6 more vertex that makes 2 GL_TRIANGLE_STRIP
    // that´s going to be the frame for the pixels of the sparkImg texture
    //
    //for(int i = 0; i < gl_in.length(); i++){
        gl_Position = gl_in[0].gl_Position  + vec4(-radius,-radius,0.0,0.0);
        vTexCoord.x = 0.0;
        vTexCoord.y = 0.0;
        EmitVertex();
        
        gl_Position = gl_in[0].gl_Position  + vec4(radius,-radius,0.0,0.0);
        vTexCoord.x = imgWidth;
        vTexCoord.y = 0.0;
        EmitVertex();
        
        gl_Position = gl_in[0].gl_Position + vec4(radius,radius,0.0,0.0);
        vTexCoord.x = imgWidth;
        vTexCoord.y = imgHeight;
        EmitVertex();
        EndPrimitive();

        gl_Position = gl_in[0].gl_Position + vec4(-radius,-radius,0.0,0.0);
        vTexCoord.x = 0.0;
        vTexCoord.y = 0.0;
        EmitVertex();
        EndPrimitive();
    //}
}
