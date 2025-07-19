#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 8) out;


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


        // draw the shaft of the arrow:
        fColor = gs_in[0].color_field;
        gl_Position = tail;// + vec4(-Arrowlenght, -Arrowlenght, 0.0, 0.0); // 1:bottom-left
        EmitVertex();
        gl_Position =   head; // 2:bottom-right
        EmitVertex();
        //gl_Position = position + vec4(-Arrowlenght,  Arrowlenght, 0.0, 0.0); // 3:top-left
        //EmitVertex();
        //gl_Position = position + vec4( Arrowlenght,  Arrowlenght, 0.0, 0.0); // 4:top-right
        //EmitVertex();
        //gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0); // 5:top
        //fColor = vec4(1.0, 1.0, 1.0,1.0);
        //EmitVertex();
        //EndPrimitive();


    /*    gl_Position = vec4(tail,0.0);
        EmitVertex();
        gl_Position = vec4(head,0.0);
        EmitVertex();
*/

        // draw two sets of wings in the non-major directions:


        vec4 dir;
        if( axis != XX )
        {
                v=vec4(cross( normalize(vec3(w)), axx ),0.0);
                normalize(v);
                u=vec4(cross( vec3(v), normalize(vec3(w))),0.0);
                dir = u-w;
                normalize(dir);

                vec4 tmp = head + d*Arrowlenght*dir;
                gl_Position = head;
                EmitVertex();
                gl_Position = tmp;
                EmitVertex();
                dir =-u-w;
                normalize(dir);
                vec4 tmp1 = head + d*Arrowlenght*dir;
                gl_Position = head;
                EmitVertex();
                gl_Position = tmp1;
                EmitVertex();

        }


       if( axis != YY )
        {
           v=vec4(cross( normalize(vec3(w)), ayy ),0.0);
           normalize(v);
           u=vec4(cross( vec3(v), normalize(vec3(w))),0.0);
           dir = u-w;
           normalize(dir);

           vec4 tmp2 = head + d*Arrowlenght*dir;
           gl_Position = head;
           EmitVertex();
           gl_Position = tmp2;
           EmitVertex();
           dir =-u-w;
           normalize(dir);
           vec4 tmp3 = head + d*Arrowlenght*dir;
           gl_Position = head;
           EmitVertex();
           gl_Position = tmp3;
           EmitVertex();


        }

       if( axis != ZZ )
        {
           v=vec4(cross( normalize(vec3(w)), azz ),0.0);
           normalize(v);
           u=vec4(cross( vec3(v), normalize(vec3(w))),0.0);
           dir = u-w;
           normalize(dir);

           vec4 tmp4 = head + d*Arrowlenght*dir;
           gl_Position = head;
           EmitVertex();
           gl_Position = tmp4;
           EmitVertex();
           dir =-u-w;
           normalize(dir);
           vec4 tmp5 = head + d*Arrowlenght*dir;
           gl_Position = head;
           EmitVertex();
           gl_Position = tmp5   ;
           EmitVertex();


        }


        EndPrimitive();

}







void main() {    
    //build_house(gl_in[0].gl_Position);
    DrawArrow(gl_in[0].gl_Position);
}
