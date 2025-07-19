#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 5) out;


uniform float radius;
uniform float min_scalar;
uniform float max_scalar;
uniform bool vok;
in VS_OUT {

        flat vec4 color_field;
        flat vec3 vel_field;
}gs_in[];

out vec4 fColor;
out vec3 f_vel;

float norm(vec3 v)
{
    return sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}

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



void
DrawArrow(vec4 position)
{



   vec3 axx=vec3 (1.0, 0.0, 0.0);
 vec3 ayy = vec3( 0.0, 1.0, 0.0);
 vec3 azz= vec3(0.0, 0.0, 1.0 );
  vec4 tail, head;
  vec3 tmpd=gs_in[0].vel_field;
  f_vel=tmpd;
   float t = length(tmpd);
  float k =(t-min_scalar)/(max_scalar-min_scalar);
  normalize(tmpd);
  float Arrowlenght =1.0f*radius+ k*2*radius;
  tail=position;
  head[0]=tail[0]+Arrowlenght*tmpd[0];head[1]=tail[1]+Arrowlenght*tmpd[1];head[2]=tail[2]+Arrowlenght*tmpd[2];head[3]=tail[3]+0.0;
        vec4 u, v, w;		// arrow coordinate system

        // set w direction in u-v-w coordinate system:

        w[0] = head[0] - tail[0];
        w[1] = head[1] - tail[1];
        w[2] = head[2] - tail[2];
        w[3] = head[3] - tail[3];

        // determine major direction:
        int XX=1;
        int YY=2;
        int ZZ=3;
        int axis = XX;
        float mag = abs( w[0] );
        if(  abs( w[1] )  > mag  )
        {
                axis = YY;
                mag = abs( w[1] );
        }
        if(  abs( w[2] )  > mag  )
        {
                axis = ZZ;
                mag = abs( w[2] );
        }


        // set size of wings and turn w into a Unit vector:
        //float WINGS = 0.01;

        float d = 0.25;
        if(vok)
        {
            float nv=min(k, 1.0);
            nv = min(0.72*nv, 0.72);
            vec3 fluidColor =hsv2rgb(vec3(0.72- nv, 1.0f, 1.0f));
            fColor = vec4(fluidColor,gs_in[0].color_field[3]);
        }
        else
            fColor= gs_in[0].color_field;

        // draw the shaft of the arrow:

        gl_Position = tail + vec4(-Arrowlenght, -Arrowlenght, 0.0, 0.0); // 1:bottom-left
        EmitVertex();
        gl_Position =   tail + vec4(Arrowlenght, -Arrowlenght, 0.0, 0.0); // 1:bottom-left
        EmitVertex();
        gl_Position = position + vec4(-Arrowlenght,  Arrowlenght, 0.0, 0.0); // 3:top-left
        EmitVertex();
        gl_Position = position + vec4( Arrowlenght,  Arrowlenght, 0.0, 0.0); // 4:top-right
        EmitVertex();
        //gl_Position = position; // 5:top
        //fColor = vec4(1.0, 1.0, 1.0,1.0);
        //EmitVertex();
        EndPrimitive();






}







void main() {    
    //build_house(gl_in[0].gl_Position);
    DrawArrow(gl_in[0].gl_Position);
}
