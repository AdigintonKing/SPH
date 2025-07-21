#ifndef SPH_H
#define SPH_H

/*#include "CImg.h"
using namespace cimg_library;
#undef min
#undef max
#define cimg_imagepath "./../img/"
*/

#include <omp.h>
#include <chrono>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ColorRGBA.hpp"
#include "Cores.h"
#include "NeighborhoodSearch.h"
#include "Utilities/vector.hh"
#include "neighbordata.hh"
#include "resources/colormaps/colormap_jet.h"
//#include "kdtree.hh"
//#include "hashgrid.hh"
#include <stb_image.h>
#include <glad/glad.h>
#include "lgl/lgl.h"
//#include <GL/glew.h>
//#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
// we need vec3 rotation
#include "glm/gtx/vector_angle.hpp"
//#include <glm/glm.hpp>
//#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/shader.h>
//#include <learnopengl/shader_s.h>
#include <learnopengl/camera.h>
#include <learnopengl/filesystem.h>
#include<stb_image_writer.h>
#include "Utilities/AVX_math.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <CGAL/Timer.h>
#include <CGAL/Memory_sizer.h>
#include <Magick++.h>
//Includes para nuvens de ponto
//#include <opencv2/core/core.hpp>
//#include <opencv2/opencv.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include "PLYData.hpp"
//#include "ObjectDetector.hpp"
//#include "plycpp.cpp"
///#include"plycpp.h"
//#include <string>
//#include <filesystem>
//#include <array>

#include <iostream>


using namespace std;
using namespace Magick;
//using namespace cv; // Leitor PLY

glm::vec3 cameraInitialPos = glm::vec3(1.0f, 1.0f, 2.5f);
glm::vec3 cameraInitialFront = glm::vec3(1.0f, 1.0f, -1.0f);
glm::vec3 cameraInitialUp = glm::vec3(0.0f, 1.0f,  0.0f);
glm::vec3 cameraPos;
glm::vec3 cameraAuxPos;
glm::vec3 cameraFront = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
glm::mat4 view;
float yaw   = 0.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float fov=45.0f;
bool woodBox=true;
bool skyBox=false;
bool imgok=false;
bool wireframeMode=false;
float deltaTime ;	// time between current frame and last frame
float lastFrame;
bool firstMouse;
float lastX,lastY;
bool mouse_left;
bool mouse_right;
Camera *camera;
GLFWwindow* window;
glm::mat4 transform;
glm::mat4 model;
glm::vec3 planeRotXYZ = glm::vec3(0.0f);
glm::mat3 orientation;
// shaders

Shader instanceShader;
Shader m_shader_scalar;
Shader sphereShader;
Shader vectorFieldShader;
Shader vectorFieldShaderColor;
Shader ScreenBuffer;
class Texture2D
{
public:
    // holds the ID of the texture object, used for all texture operations to reference to this particlar texture
    unsigned int ID;
    // texture image dimensions
    unsigned int Width, Height; // width and height of loaded image in pixels
    // texture Format
    unsigned int Internal_Format; // format of texture object
    unsigned int Image_Format; // format of loaded image
    // texture configuration
    unsigned int Wrap_S; // wrapping mode on S axis
    unsigned int Wrap_T; // wrapping mode on T axis
    unsigned int Filter_Min; // filtering mode if texture pixels < screen pixels
    unsigned int Filter_Max; // filtering mode if texture pixels > screen pixels
    // constructor (sets default texture modes)
    Texture2D();
    void loadTextureFromFile(const char *file, bool alpha);
    bool load_texture(const char *file_name);
    // generates texture from image data
    void Generate(unsigned int width, unsigned int height, unsigned char* data);
    // binds the texture as the current active GL_TEXTURE_2D texture object
    void Bind() const;
};


#define soundwater 1450.0
//##################################################################//


#define FORALL_FLUID_NEIGHBORS(CODE) \
        for (unsigned int j = 0; j < neighbors[i].size(); j++) \
        { \
             const NeighborData &data = neighbors[i][j]; \
            CODE \
        } \


/** Loop over the fluid neighbors of the same fluid phase.
* Simulation *sim, unsigned int fluidModelIndex and FluidModel* model must be defined.
*/
#define forall_fluid_neighbors_avx(code) \
    const unsigned int maxN = m_neighborhoodSearch->point_set(0).n_neighbors(0, i); \
    for (unsigned int j = 0; j < maxN; j += 8) \
    { \
        const unsigned int count = std::min(maxN - j, 8u); \
        const Vector3f8 xj_avx = convertVec_zero(&m_neighborhoodSearch->point_set(0).neighbor_list(0,i)[j], &Points_positions[0], count); \
        code \
    }




class Particles {
public:
  // position
  std::vector<std::array<float, 3>> x;
  // velocity
  std::vector<std::array<float, 3>> v;
  
  // acceleration acting on this particle in this step
  std::unique_ptr<glm::vec4[]> a;
  // smoothed velocity for artificial viscosity
  std::unique_ptr<glm::vec4[]> v_smoothed;
  // density
  std::vector<float> rho;
  // density
  std::vector<float> rho_squared;
  // pressure
  std::vector<float> p;
    // Temperature
  std::vector<float> T;
  // temperature gradient
  std::vector<float> dt;
  std::vector<std::array<float, 4>> C;

  std::unique_ptr<bool[]>  m_alive;

  size_t m_count{ 0 };
  size_t m_countAlive{ 0 };
  void kill(size_t id);
  void wake(size_t id);
  void swapData(size_t a, size_t b);
  
  Particles() {
   //T=Tini;
  }
};

void Particles::kill(size_t id)
{
    if (m_countAlive > 0)
    {
        m_alive[id] = false;
        swapData(id, m_countAlive - 1);
        m_countAlive--;
    }
}


void Particles::wake(size_t id)
{
    if (m_countAlive < m_count)
    {
        m_alive[id] = true;
        swapData(id, m_countAlive);
        m_countAlive++;
    }
}
void Particles::swapData(size_t aa, size_t bb)
{
    std::swap(x[aa], x[bb]);
    std::swap(v[aa], v[bb]);
    std::swap(a[aa], a[bb]);
    std::swap(v_smoothed[aa], v_smoothed[bb]);
    std::swap(m_alive[aa], m_alive[bb]);
}

inline float rand01()
{
  return float(rand())/RAND_MAX;
}

inline glm::vec3 randomDirection()
{
  float alpha = 2 * M_PI * rand01();
  float beta = 2 * M_PI * rand01();
  
  return glm::vec3(cos(alpha)*cos(beta), sin(alpha)*cos(beta), sin(beta));
}

struct std_kernel {
  float h, h_squared, h_third, h_fourth,pi_overFour_h_squared;
  
  std_kernel(float h_): h(h_), h_squared(h*h), pi_overFour_h_squared(M_PI*h_squared/4.f), h_third(h_squared*h), h_fourth(M_PI*h_squared*h_squared/24.f) {
  }
  
  inline float value(float d, float d_squared) const {
   // if (d_squared >= h_squared)
     // return 0.f;
    //else {
      float x = 1.f - d_squared/h_squared;
      return 1.f/(pi_overFour_h_squared) * x * x * x;
    //}
  }
  
  inline float first_derivative(float d, float d_squared) const {
    /*if (d >= h)
      return 0.f;
    else {*/
      const Real x = 1.f - d_squared/h_squared;
      const Real resp = -d/(h_fourth) * x * x;
      return resp;
    //}
  }
  
  inline glm::vec3 gradient(NeighborData const &nd) const {
    return - first_derivative(nd.d, nd.d_squared) * nd.d_normalized;
  }

  inline float second_derivative(float d, float d_squared) const {
    /*if (d_squared >= h_squared)
      return 0.f;
    else {*/
      float x = d_squared/h_squared;
      return 24.f / (M_PI*h_fourth) * (1 - x) * (5 * x - 1);
    //}
  }
};

struct std_kernel_avx {
  Scalarf8 h, h_squared, h_third, h_fourth,pi_overFour_h_squared;
  Scalarf8 h_squared_avx;

  std_kernel_avx(float h_): h(h_), h_squared(h_*h_),h_squared_avx(h_*h_), pi_overFour_h_squared(1.0f/(M_PI*h_*h_/4.f)), h_third(h_squared*h_), h_fourth(M_PI*h_*h_*h_*h_/24.f) {
  }

  inline Scalarf8 value(const Scalarf8 &d, const Scalarf8 &d_squared) const {
   // if (d_squared >= h_squared)
     // return 0.f;
    //else {
      const Scalarf8 uno(1.0f);
      const Scalarf8 x =uno - d_squared/h_squared_avx;
      return  x * x * x*pi_overFour_h_squared;
    //}
  }

  inline Scalarf8 first_derivative(Scalarf8 &d, Scalarf8 &d_squared) const {
    /*if (d >= h)
      return 0.f;
    else {*/
      const Scalarf8 uno(1.0f);
      const Scalarf8 x  =(uno - d_squared/h_squared_avx);
      //Real t = x.reduce();
      const Scalarf8 resp = -d/(h_fourth) * x * x;
      //t=resp.reduce();
      return blend(d_squared < h_squared, resp, Scalarf8(0.0f));
    //}
  }

  inline Vector3f8 gradient(const Vector3f8 &delta_x) const {
     Vector3f8 v = delta_x;
     Scalarf8 d_squared = v.squaredNorm();
     Scalarf8 d = d_squared.sqrt();
     v.normalize();
    return v*first_derivative(d, d_squared);
  }

  inline Scalarf8 second_derivative(Scalarf8 &d, Scalarf8 &d_squared) const {
    /*if (d_squared >= h_squared)
      return 0.f;
    else {*/
      const Scalarf8 uno(1.0f);
      const Scalarf8 x=d_squared/h_squared;
      return (uno - x) * ( x*5.0 - uno);
    //}
  }
};

struct spiky_kernel {
  float h, h_squared, h_third, h_fourth,pi_overTen_h_squared;
  
  spiky_kernel(float h_): h(h_), h_squared(h*h), pi_overTen_h_squared(1.0f/(M_PI*h_squared*0.1)), h_third(M_PI*h_squared*h/30.f), h_fourth(h_squared*h_squared) {
  }

  inline float value(float d, float d_squared) const {
    if ((d >= h))
      return 0.f;
    else {
       float t2=pi_overTen_h_squared;
      float x = 1.0f - d/h;
      return x * x * x*(pi_overTen_h_squared)  ;
    }
  }
  
  inline float first_derivative(float d, float d_squared) const {
    if ((d >= h)||(d==0.0))
      return 0.f;
    else {
      const Real x = 1.f - d/h;
      const Real resp = -1.f / (h_third) * x * x;
      return resp;
    }
  }
  
  inline glm::vec3 gradient(NeighborData const &nd) const {
    return - first_derivative(nd.d, nd.d_squared) * nd.d_normalized;
  }
  
  inline float second_derivative(float d, float d_squared) const {
    /*if ((d >= h)||(d==0.0))
      return 0.f;
    else {*/
      float x = 1.f - d/h;
      return 60.f / (M_PI*h_fourth) * x;
    //}
  }
};


struct spiky_kernel_avx {
  Scalarf8 h, h_squared, h_third, h_fourth,pi_overTen_h_squared;

  spiky_kernel_avx(float h_): h(h_), h_squared(h_*h_), pi_overTen_h_squared(M_PI*h_*h_*static_cast<Real>(0.1)), h_third(-1.0f/(h_*h_*h_*M_PI/30.f)), h_fourth(h_squared*h_squared) {
  }

  inline Scalarf8 value(const Scalarf8 &d) const {
    /*if ((d >= h)||(d==0.0))
      return 0.f;
    else {*/
       //Real t2,x1;
      const Scalarf8 uno(static_cast<Real>(1.0));
      //std::cout <<"( "<< uno.v[0] << ", "<< uno.v[1] << ", "<< uno.v[2] << ", "<< uno.v[3] << ", "<< uno.v[4] << ", "<< uno.v[5] << ", "<< uno.v[6] << ", "<< uno.v[7] << " )"<<std::endl;

      const Scalarf8 x=uno-d/h;
      const Scalarf8 res = (x * x * x)/pi_overTen_h_squared;
      //const Scalarf8 x(t1);
      //x1= x.reduce();
      //t2 = pi_overTen_h_squared;
      //std::cout <<"( "<< x.v[0] << ", "<< x.v[1] << ", "<< x.v[2] << ", "<< x.v[3] << ", "<< x.v[4] << ", "<< x.v[5] << ", "<< x.v[6] << ", "<< x.v[7] << " )"<<std::endl;
       return blend(d < h, res, Scalarf8(0.0f));
    //}
  }

  inline Scalarf8 first_derivative(Scalarf8 &d, Scalarf8 &d_squared) const {
    /*if ((d >= h)||(d==0.0))
      return 0.f;
    else {*/
      const Scalarf8 uno(static_cast<Real>(1.0));
      //Real t=uno.reduce();
      const Scalarf8 x = uno - d/h;
      //t= x.reduce();
      const Scalarf8 resp = h_third * x * x;
      //t= resp.reduce();
      return blend(d < h, resp, Scalarf8(0.0f));
    //}
  }

  inline Vector3f8 gradient(const Vector3f8 &delta_x) const {
        Vector3f8 v = delta_x;
        Scalarf8 d_squared = v.squaredNorm();
        Scalarf8 d = d_squared.sqrt();
        v.normalize();
       return v*first_derivative(d, d_squared);

  }

  inline Scalarf8 second_derivative(Scalarf8 &d, Scalarf8 &d_squared) const {
    /*if ((d >= h)||(d==0.0))
      return 0.f;
    else {*/
      Scalarf8 x(1.f - d/h);
      return Scalarf8(60.f/(h_fourth)) * x;
    //}
  }
};

struct Parameters {
  // time step
  float dt;
  
  // sph internal
  float rel_smoothing_radius;
  
  // constants for compressibility model
  int gamma;
  float K;
  
  // rest density of fluid
  float rho0;
  
  // fraction of pressure to apply for negative pressures
  float zeta;
  
  // viscosity parameters
  float nu;
  // artificial viscosity
  float xi;
  bool real_xsph;
  
   // environment
  glm::vec4 gravity;
  // restitution
  float a;  
  //thermal diffusion constant
  float alpha;
  int steps;
  float frame_dt;
};

class SPH {
  // creation parameters
  float jitter;
  float spacing;
  // effective smoothing radius (computed from spacing)

  // mass of each particle (computed from density and particle spacing)
  float m;

   Scalarf8 const_Viscosity;
  Scalarf8 h_squared;
  
  // cell size for surface extraction
  float isovalue;
  int xsurfacecells, ysurfacecells;
  float cellsize;

  // particle store
  
  
  // spatial search data structures
  
  std::vector<std::vector<NeighborData> > neighbors;
    
  // function mapping density to pressure
  inline float pressure(float rho) {
    float p = params.K * (pow((rho/params.rho0 ), params.gamma)- 1.f)+params.rho0;      
    if (p < 0)
      p *= params.zeta;
    return p;
  }
  
public:
  Parameters params;
float h;
std::string screenFilename;
int winWidth, winHeight ;
int screenFilenameNumber;
long int n_step;
long int steps;
float gridVertices[948];
unsigned int gridIndices[316];
std::vector<glm::vec3> curveVertex;
std::vector<glm::vec3> polygonVertex;
std::vector<unsigned int> curveIndex;
std::vector<unsigned int> polygonIndex;
// KdTree tree;
  //Hashgrid;
  // kernels
  typedef spiky_kernel pressure_kernel;
  typedef std_kernel viscosity_kernel;
typedef spiky_kernel_avx pressure_kernel_avx;
typedef std_kernel_avx viscosity_kernel_avx;
  float Tmax,Tmin;
  Particles particles;
  pressure_kernel pkernel;
  viscosity_kernel vkernel;
  pressure_kernel_avx pkernelAvx;
  viscosity_kernel_avx vkernelAvx;
  NeighborhoodSearch *m_neighborhoodSearch;
  bool tinit=false;
  bool pforce=false;
  float time=0.0f;
 std::chrono::duration<double, std::milli> tsum;
 std::chrono::duration<double, std::milli> tforce;
 std::chrono::duration<double, std::milli> tcollision;
 std::chrono::duration<double, std::milli> trender;
 std::chrono::duration<double, std::milli> tpressure;
 std::chrono::duration<double, std::milli> Trender;

float NormMaxV=-100000.0,NormMinV=10000.0, Vmax;
int NormMaxVid;
  SPH(): h(0), pkernel(h), vkernel(h), pkernelAvx(h),vkernelAvx(h) {
    screenFilenameNumber=0;n_step=0;steps=0;
winWidth =1000; winHeight =600;
screenFilename = "screenVis";
    // set default parameter values
    
    // sph internals
    params.rel_smoothing_radius = 2.5;

    // material parameters
    params.rho0 = 998.6;
    params.K = 1200	;
    params.nu = 0.04; // 0.002 is a good value if used alone
    params.xi = 0.1; // 0.01 is a good value if used alone
    params.real_xsph = true; // real XSPH (smoothed velocity only used in integration) is much weaker
    params.steps=0;
    // pressure computation
    params.zeta = 0.5;
    params.gamma = 4;
    
    // environment
    params.gravity = glm::vec4(0, -9.8, 0,0);
    //params.gravity = glm::vec4(0, 0.0, 0,0);

    params.a = 0.7;
        
    // time step
    params.dt = 0.001;
    
    // surface extraction
    isovalue = 0.5;
    surfaceCells(80);
    params.alpha = 0.0;
    params.frame_dt=0.01 ;
  }  

  inline void parameters(Parameters const &pm) {
    bool set_r = params.rel_smoothing_radius != pm.rel_smoothing_radius;
    bool set_m = params.rho0 != pm.rho0;
    params = pm;
    if (set_r) 
      smoothing_radius(spacing * params.rel_smoothing_radius);
    else if (set_m) 
      compute_mass();
  }
  
  std::vector<glm::vec3> sample_hex(float spacing, float jitter, Vector min, Vector max) {
    const float spacing_2 = spacing/2;
    const float yspacing = spacing*sqrt(3.0);
    const float yspacing_2 = yspacing/2;
    
    glm::vec3 pos;
    std::vector<glm::vec3> positions;
    bool yraised = false;
    for (pos.x = min.x; pos.x <= max.x; pos.x += spacing_2)
    {
      yraised = !yraised;
      if (yraised)
        pos.y = min.y + yspacing_2;
      else
        pos.y = min.y;
      
      while (pos.y <= max.y)
      {
        glm::vec3 p = pos;
        
        if (jitter != 0.0f) {
          p += jitter*spacing*rand01()*randomDirection();

        }
        p.z = 0.0;
        positions.push_back(p);
        pos.y += yspacing;
      }
    }
    
    return positions;
  }

  std::vector<glm::vec3> sample_hex_cloud(float spacing, float jitter, Vector min, Vector max, std::vector<glm::vec4> *Vcolor) {

      glm::vec3 pos;
      std::vector<glm::vec3> positions;
      std::vector< glm::vec4 > temp_color;
      std::vector< glm::vec3 > temp_normals;

      FILE * file = fopen("./../clouds/pedra.ply", "r");
      if( file == NULL ){
          printf("Impossible to open the file !\n");
          return positions;
      }

      Color vertex_color;
      int checker;
      checker = 0;
      int loop = 1;

      while( 1 ){

          char lineHeader[128];
          int res = fscanf(file, "%s", lineHeader);
          //float res1 = std::stof(res);
          if (res == EOF)
              break;

           if ( strcmp( lineHeader, "end_header" ) == 0 ){
              checker = 1;

          }else if (checker ==1){

            glm::vec3 vertex;
            glm::vec3 normals;
            glm::vec4 color;

            std::string fs(lineHeader);
            float f=std::stof(fs);//this is much better way to do it

            vertex.x = f;

            fscanf(file, "%f %f %f %f %f %f %f %f \n", &vertex.y, &vertex.z, &normals.x, &normals.y, &normals.z, &color.x, &color.y, &color.z);

            color[0] = color[0] / 255;
            color[1] = color[1] / 255;
            color[2] = color[2] / 255;
            color[3] = 1.0;
            positions.push_back(vertex);
            temp_normals.push_back(normals);
            Vcolor->push_back(color);

            std::cout << loop << "(" << vertex.x << "," << vertex.y << "," << vertex.z <<") ="
                      << " R " << color[0]
                      << " G " << color [1]
                      << " B " << color[2]<< std::endl;

            loop++;
          }
        }

      return positions;
  }


  std::vector<glm::vec3> sample_hex_Image(float &spacing, float jitter, Vector min, Vector max,std::vector<glm::vec4> *Vcolor) {
       imgok=true;
      Image image;
      //image.read("./../img/usp_each.png");
      //image.read("./../img/Antonio.png");
      //image.read("./../img/tcc.png");
      //image.read("./../img/heitor.png");
      //image.read("./../img/Amanda2.png");
      //image.read("./../img/Fig.png");
      //image.read("./../img/Edsandra.png");
      //image.read("./../img/Ivan.png");
      //image.read("./../img/Aline.png");
      //image.read("./../img/Helton.png");
      //image.read("./../img/Fatima.png");
      //image.read("./../img/Frank.png");
      //image.read("./../img/Elenir.png");
      image.read("./../img/Lapis.png");
      //image.read("./../img/Ana.png");
     // CImg<double> image("./../img/heitor.png");
      //CImg<double> image("./../img/Ana.png");
     int w =image.columns();
     int h = image.rows();

     Color pixel_sample;

    std::cout << w << " x " << h << std::endl;
     /*for (int r = 0; r < h; r++)
         for (int c = 0; c < w; c++)
         {
             pixel_sample= image.pixelColor(r,c);
             std::cout << "(" << r << "," << c << ") ="
                  << " R " << (float)(pixel_sample.quantumRed()/65536)
                  << " G " << (float)(pixel_sample.quantumGreen()/65536)
                  << " B " << (float)(pixel_sample.quantumBlue()/65536) << std::endl;
         }*/


    const float spacing_2 = (max.x-min.x)/w;

    const float yspacing = spacing*sqrt(3.0);
    const float yspacing_2 = (max.y-min.y)/h;
    spacing = 1.0*(spacing_2+yspacing_2)*0.5;
    glm::vec4 ci;
    int c=0, r=0;
    glm::vec3 pos;
    std::vector<glm::vec3> positions;
    bool yraised = false;
    pos.y = min.y;
    while (pos.y < max.y)
    {

      yraised = !yraised;
      //if (yraised)
        //pos.y = min.y + yspacing_2;
      //else
        pos.x = min.x;
      r=0;
      while (pos.x < max.x)
      {
        glm::vec3 p = pos;

        if (jitter != 0.0f) {
          p += jitter*spacing*rand01()*randomDirection();
          p.z = 0.0;
        }

        /*std::cout << "(" << r << "," << h-c-1 << ") ="
                << " R " << (float)(pixel_sample.quantumRed()/65536)
                << " G " << (float)(pixel_sample.quantumGreen()/65536)
                << " B " << (float)(pixel_sample.quantumBlue()/65536) << std::endl;*/
        pixel_sample= image.pixelColor(r,h-c-1);
        ci[0] = (float)(pixel_sample.quantumRed()/65536);
        ci[1] = (float)(pixel_sample.quantumGreen()/65536);
        ci[2] = (float)(pixel_sample.quantumBlue()/65536);
        ci[3]=1.0;
        Vcolor->push_back(ci);
        positions.push_back(p);
        pos.x += spacing_2;
        r++;
      }
     // if(c==2)
      //break;
      pos.y+=yspacing_2; c++;
    }

    return positions;

}

void initFromCloud(float jitter = 0.02, float spacing = 0.015){
    this->jitter = jitter;
    this->spacing = spacing;
    Vector min = Vector(-0.69,0.5,0);
    Vector max = Vector(0.91, 1.9, 0);
    std::vector<glm::vec4> Vcolor;
    std::vector<glm::vec3> pos = sample_hex_cloud(this->spacing, jitter, min, max,&Vcolor);

    particles.m_count = pos.size();
    steps=0;
    //particles.clear();

    particles.x.resize(pos.size());

    particles.T.resize(pos.size());
    //particles.a.resize(pos.size());
    particles.rho.clear();
    particles.a.reset(new glm::vec4[pos.size()]);
    particles.rho.resize(pos.size());
    particles.rho_squared.resize(pos.size());
    particles.v_smoothed.reset(new glm::vec4[pos.size()]);
    particles.v.resize(pos.size());
    particles.p.resize(pos.size());
    particles.dt.resize(pos.size());
    particles.C.resize(pos.size());

    for (int i = 0; i < pos.size(); i++) {
        particles.x[i][0] =pos[i].x; particles.x[i][1] =pos[i].y; particles.x[i][2] =pos[i].z;// glm::vec4(pos[i].x,pos[i].y,pos[i].z,1.0);
        particles.v[i]={0.0,0.0,0.0};
        particles.a[i] = glm::vec4(0.0);
        particles.C[i][0]= Vcolor[i][0];
        particles.C[i][1]= Vcolor[i][1];
        particles.C[i][2]= Vcolor[i][2];
        particles.C[i][3]= Vcolor[i][3];
    }

    int q = smoothing_radius(this->spacing * params.rel_smoothing_radius);
    // reserve memory for  and compute
    neighbors.resize(n_particles());
    // update search data structures
    updateSearcher();
    //updateSearcher(grid);


    for (int i = 0; i < n_particles(); ++i) {
        //neighbors[i].reserve(2*q);
        particles.T[i]=25.0; particles.dt[i]=0.0;

        //tree.neighbors(particles[i].x, neighbors[i]);
    }
    Tmax=25.0;Tmin=25.0;
    const_Viscosity = Scalarf8(2.0*params.nu * m);
    std::cout << "created " << n_particles() << " particles." << std::endl;
}


void initFromImage(float jitter = 0.02, float spacing = 0.015) {
    this->jitter = jitter;
    this->spacing = spacing;
    Vector min = Vector(-0.69,0.5,0);
    Vector max = Vector(0.91, 1.9, 0);
    std::vector<glm::vec4> Vcolor;
    // fill half a [0,1]^2 box with particles
    std::vector<glm::vec3> pos = sample_hex_Image(this->spacing, jitter, min, max,&Vcolor);
    particles.m_count = pos.size();
    steps=0;
    //particles.clear();

    particles.x.resize(pos.size());

    particles.T.resize(pos.size());
    //particles.a.resize(pos.size());
    particles.rho.clear();
    particles.a.reset(new glm::vec4[pos.size()]);
    particles.rho.resize(pos.size());
    particles.rho_squared.resize(pos.size());
    particles.v_smoothed.reset(new glm::vec4[pos.size()]);
    particles.v.resize(pos.size());
    particles.p.resize(pos.size());
    particles.dt.resize(pos.size());
    particles.C.resize(pos.size());

    for (int i = 0; i < pos.size(); i++) {
      particles.x[i][0] =pos[i].x; particles.x[i][1] =pos[i].y; particles.x[i][2] =pos[i].z;// glm::vec4(pos[i].x,pos[i].y,pos[i].z,1.0);
      particles.v[i]={0.0,0.0,0.0};
      particles.a[i] = glm::vec4(0.0);
      particles.C[i][0]= Vcolor[i][0];
      particles.C[i][1]= Vcolor[i][1];
      particles.C[i][2]= Vcolor[i][2];
      particles.C[i][3]= Vcolor[i][3];

    }


    int q = smoothing_radius(this->spacing * params.rel_smoothing_radius);
    // reserve memory for  and compute
    neighbors.resize(n_particles());
    // update search data structures
    updateSearcher();
    //updateSearcher(grid);


    for (int i = 0; i < n_particles(); ++i) {
      //neighbors[i].reserve(2*q);
      particles.T[i]=25.0; particles.dt[i]=0.0;

      //tree.neighbors(particles[i].x, neighbors[i]);
    }
    Tmax=25.0;Tmin=25.0;
    const_Viscosity = Scalarf8(2.0*params.nu * m);
    std::cout << "created " << n_particles() << " particles." << std::endl;
  }

  void init(float jitter = 0.02, float spacing = 0.012) {
    this->jitter = jitter;
    this->spacing = spacing;

    //std::vector<glm::vec4> Vcolor; //Para Nuvem
    //std::string caminhoArquivo = "./../clouds/pedra.ply";
    //auto nuvem = lerNuvemDePontos(caminhoArquivo);

    
    // fill half a [0,1]^2 box with particles
    std::vector<glm::vec3> pos = sample_hex(spacing, jitter, Vector(-0.69,0.7,0), Vector(0.09, 2.3, 0));
    //std::vector<glm::vec3> pos = sample_hex_cloud(spacing, jitter, Vector(-0.69,0.7,0), Vector(0.09, 2.3, 0), &Vcolor); //Nuvem
    //std::vector<glm::vec3> pos = sample_hex(spacing, jitter, Vector(-1.69,0.7,0), Vector(1.09, 2.3, 0));
    steps=0;
    particles.m_count = pos.size();
    //particles.clear();

     particles.x.resize(pos.size());

    particles.T.resize(pos.size());
    //particles.a.resize(pos.size());
    particles.rho.clear();
    particles.a.reset(new glm::vec4[pos.size()]);
    particles.rho.resize(pos.size());
    particles.rho_squared.resize(pos.size());
    particles.v_smoothed.reset(new glm::vec4[pos.size()]);
    particles.v.resize(pos.size());
    particles.p.resize(pos.size());
    particles.dt.resize(pos.size());
    particles.C.resize(pos.size());

    for (int i = 0; i < pos.size(); i++) {
      particles.x[i][0] =pos[i].x; particles.x[i][1] =pos[i].y; particles.x[i][2] =pos[i].z;//glm::vec4(pos[i].x,pos[i].y,pos[i].z,1.0);;
      particles.v[i] = {0.0,0.0,0.0};
      particles.a[i] = glm::vec4(0.0);
      particles.C[i][0]= blue.R;
      particles.C[i][1]= blue.G;
      particles.C[i][2]= blue.B;
      particles.C[i][3]= blue.A;


    }
        
    int q = smoothing_radius(spacing * params.rel_smoothing_radius);
    // reserve memory for  and compute
    neighbors.resize(n_particles());
    // update search data structures
    updateSearcher();
    //updateSearcher(grid);


    for (int i = 0; i < n_particles(); ++i) {
      //neighbors[i].reserve(2*q);
      particles.T[i]=25.0; particles.dt[i]=0.0;
      
      //tree.neighbors(particles[i].x, neighbors[i]);
    }
    Tmax=25.0;Tmin=25.0;
    const_Viscosity = Scalarf8(2.0*params.nu * m);
    std::cout << "created " << n_particles() << " particles." << std::endl;
  }
  void NormalizedColor(float T,TColorRGBA *C)
  {
           if((Tmax - Tmin)>0.01)
	   {
            float normalizedV = 2*((T-Tmin)/(Tmax-Tmin))-1.0;
             float jump =floor((1.0+normalizedV)/0.05);
                        if(normalizedV<0.0)
                        {
                                C->R=15/255;
                                C->G = (15+6*jump)/255;
                                C->B = (255-6*jump)/255;
                                
                        }
                        else
                        {
                                C->R = (15+6*(jump))/255;
                                C->G =(255- 6*(jump))/255;
                                C->B=15/255;
                        }
	   }
       else
	   {
	     C->R = 0.0;
         C->G = 0.0;
         C->B=1.0;
	   }
  
  }

  void selectPoints(glm::vec3 p, std::vector<NeighborData> &vn)
  {
      float px[3];
      px[0]=p.x;px[1]=p.y;px[2]=p.z;
      std::vector<std::vector<unsigned int>> v_neighbors;
      m_neighborhoodSearch->find_neighbors(px,v_neighbors);
      //std::array<float, 3> * __restrict pos = particles.x.get();
      vn.clear();
      for (int i = 0; i < v_neighbors[0].size(); ++i) {
          NeighborData nb;

          nb.d =glm::l2Norm(p-glm::vec3(particles.x[v_neighbors[0][i]][0],particles.x[v_neighbors[0][i]][1],particles.x[v_neighbors[0][i]][2]));
          nb.d_squared = nb.d*nb.d;
          nb.idx = v_neighbors[0][i];
          vn.push_back(nb);
      }
  }
  
  Scalarf8 P_ij(const Vector3f8 &delta_x,const Vector3f8 &delta_v,const Scalarf8 &rhoi_avx, const Scalarf8 &rhoj_avx)
  {

    Scalarf8 alpha (-1.0f);

    Scalarf8 a = delta_x*delta_v;
    Scalarf8 zero(0.0f);

        const Scalarf8 d_AVX = delta_x.squaredNorm();
        const Scalarf8 h_AVX(h);

      
         const Scalarf8 nu_ij=h_AVX*a*(d_AVX + h_squared);

      const Scalarf8 resp = alpha*nu_ij*Scalarf8(soundwater)/Scalarf8(0.5)*(rhoi_avx + rhoj_avx);
      return blend(a<=zero, resp, zero);

    }
	

  
  void updateSearcher() {

      auto t0 = std::chrono::high_resolution_clock::now();
      m_neighborhoodSearch->find_neighbors(true);
      std::chrono::duration<double> diff= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-t0);
      tsum +=diff;

       auto t1 = std::chrono::high_resolution_clock::now();
      #pragma omp parallel for
      for (unsigned int i = 0; i < n_particles(); ++i) {

          int nn = m_neighborhoodSearch->point_set(0).n_neighbors(0, i);
          Scalarf8 pi_rho_avx(0.0f);
          const Vector3f8 xi_avx(particles.x[i]);
          Scalarf8 d;
          Vector3f8 delta;
          //#pragma omp parallel for
          for(unsigned int j=0;j<nn;j+=8)
          {

              const uint count = std::min(nn - j, 8u);
              const Vector3f8 xj_avx = convertVec_zero(&m_neighborhoodSearch->point_set(0).neighbor_list(0,i)[j], &particles.x[0], count);
              delta = xi_avx - xj_avx;

               d = delta.norm();
              pi_rho_avx +=  pkernelAvx.value(d);

          }


          const Real rr =pi_rho_avx.reduce();
          particles.rho[i]=rr*m;
          particles.rho_squared[i] = particles.rho[i]*particles.rho[i];
          particles.p[i] = pressure(particles.rho[i]);
      }
      std::chrono::duration<double> diff1= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-t1);
      tpressure +=diff1;
  }



  
  void step() {
    // 0th pass: update search data structure (rebuild from scratch)
    updateSearcher();
    //updateSearcher(grid);
      

    
    //float p1,p2,p3;


    auto t0 = std::chrono::high_resolution_clock::now();
    // second pass: compute accelerations and smoothed velocity, add gravity
    #pragma omp parallel for
    for (unsigned int i = 0; i < n_particles(); ++i) {
      //Particle &pi = particles[i];

      particles.v_smoothed[i] = glm::vec4(0.0);
      Scalarf8 w_avx(0.0f);
      Scalarf8 zero(0.0f);
      Scalarf8 deltaT(0.0f);
      Scalarf8 p1_avx(0.0f);
      Scalarf8 p3_avx(0.0f);
      Scalarf8 FF(4.0);
      particles.dt[i]=0.0;
      const Scalarf8 mi_avx(m);
      const Scalarf8 pi_avx(particles.p[i]);
      const Scalarf8 pi_rho_avx(particles.rho[i]);
      const Scalarf8 density_i_avx(particles.rho_squared[i]);
      const Scalarf8 Ti_avx(particles.T[i]);
      Vector3f8 delta_ai(particles.a[i]);
      const Vector3f8 xi_avx(particles.x[i]);
      const Vector3f8 vi_avx(particles.v[i]);
      Vector3f8 vi_smoothed_avx(particles.v_smoothed[i]);
      int nn = m_neighborhoodSearch->point_set(0).n_neighbors(0,i);//neighbors[i].size();

        for(int j=0;j<nn;j+=8)
        {

        const uint count = std::min(nn - j, 8);
        const Vector3f8 xj_avx = convertVec_zero(&m_neighborhoodSearch->point_set(0).neighbor_list(0,i)[j], &particles.x[0], count);
        const Vector3f8 vj_avx = convertVec_zero(&m_neighborhoodSearch->point_set(0).neighbor_list(0,i)[j], &particles.v[0], count);
        const Scalarf8 pj_avx = convert_one(&m_neighborhoodSearch->point_set(0).neighbor_list(0,i)[j], &particles.p[0], count);
        const Scalarf8 density_j_avx = convert_one(&m_neighborhoodSearch->point_set(0).neighbor_list(0,i)[j], &particles.rho_squared[0], count);
        const Vector3f8 delta_x = xi_avx-xj_avx;
        const Vector3f8 PKernelGValue = pkernelAvx.gradient(delta_x);//(pkernel.gradient(data));
        const Vector3f8 VKernelGValue =vkernelAvx.gradient(delta_x);// (vkernel.gradient(data));
        delta_ai-=PKernelGValue*mi_avx*(pi_avx/(density_i_avx) + pj_avx/(density_j_avx));

        const Vector3f8 delta_v =vi_avx-vj_avx;
        const Scalarf8 Tj_avx = convert_one(&m_neighborhoodSearch->point_set(0).neighbor_list(0,i)[j], &particles.T[0], count);

        const Scalarf8 pj_rho_avx = convert_one(&m_neighborhoodSearch->point_set(0).neighbor_list(0,i)[j], &particles.rho[0], count);
        // viscosity
        //pi.a += params.nu * (pj.v - particles.v[i]) * m/(pi.rho*pj.rho) * vkernel.second_derivative(neighbors[i][j].d, neighbors[i][j].d_squared);
        //particles.a[i]+=0.125*const_Viscosity.reduce()/(particles.rho[k]) *(((particles.x[i]-particles.x[k])*vkernel.gradient(data))/(data.d_squared+h_squared.reduce()))*(particles.v[i] - particles.v[k]);
        delta_ai+=(delta_v)*((const_Viscosity/(pj_rho_avx))*(delta_x*VKernelGValue)/(delta_x.squaredNorm()+h_squared));
        // artificial viscosity
         //(teste viscosidade artificial)
        //pi.a-=((m/(pi.rho))*P_ij(pi,pj,data)*vkernel.gradient(data));
        //delta_ai-=VKernelGValue*((mi_avx/pi_rho_avx)*P_ij(delta_x,delta_v,pi_rho_avx,pj_rho_avx));
        
        const Scalarf8 wj_avx = (mi_avx/pj_rho_avx)* pkernelAvx.value(delta_x.norm());

        w_avx += wj_avx;
        vi_smoothed_avx += vj_avx*wj_avx;
        //Real wj = m/particles.rho[k] * pkernel.value(data.d, data.d_squared);
        //particles.v_smoothed[i] += particles.v[data.idx] * wj;
        //update temperature gradient;
	
    //pi.dt += (m/pj.rho)*(pi.T-pj.T)*pkernel.second_derivative(neighbors[i][j].d, neighbors[i][j].d_squared);
         p1_avx=((mi_avx/pj_rho_avx)*(FF*pi_rho_avx/(pi_rho_avx+pj_rho_avx)));
         deltaT = (Ti_avx-Tj_avx);
         p3_avx = (((delta_x)*PKernelGValue)/(delta_x.squaredNorm()+ h_squared));
         Real ddt = (p1_avx*deltaT*p3_avx).reduce();
         particles.dt[i]+=ddt;

    }
        particles.a[i].x=delta_ai.x().reduce();
        particles.a[i].y=delta_ai.y().reduce();
        //particles.a[i].z=delta_ai.z().reduce();
      //std::cout <<  particles.a[i] << std::endl;

      Real ww = w_avx.reduce();
      // normalize velocity estimate
        if(ww>0.0000001 )
        {
                particles.v_smoothed[i].x=vi_smoothed_avx.x().reduce();
                particles.v_smoothed[i].y=vi_smoothed_avx.y().reduce();
                //particles.v_smoothed[i].z=vi_smoothed_avx.z().reduce();
                particles.v_smoothed[i] /= ww;
        }

      // gravity
      particles.a[i] += params.gravity;
      if(steps<8)
         particles.a[i]+=glm::vec4(100.0,-50.0,0.0,0.0);

      if(pforce)
          particles.a[i]+=glm::vec4(12.5*pow(sin(((2.0f*M_PI)*0.125f)*time+particles.x[i][0]/25.0),5.0),0.0,0.0,0.0);
    }

NormMaxV=-100000.0;NormMinV=10000.0;

    std::chrono::duration<double> diff= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-t0);
    tforce +=diff;

    auto t1 = std::chrono::high_resolution_clock::now();
    // third pass: apply artificial viscosity, integrate, clear transient fields
#pragma omp parallel default(shared)
{
    #pragma omp for schedule(static)
    for (int i = 0; i < n_particles(); ++i) {
      //Particle &pi = particles[i];
      
      // integrate
      std::array<float, 3> ox = particles.x[i], dx;
      if (params.real_xsph) {
        particles.v[i][0] += params.dt * particles.a[i][0];
        particles.v[i][1] += params.dt * particles.a[i][1];
        particles.v[i][2] = 0.0;
        //particles.v[i]_smoothed += params.dt * pi.a;
        particles.v[i][0]=(1-params.xi) * particles.v[i][0] + params.xi * particles.v_smoothed[i][0];
        particles.v[i][1]=(1-params.xi) * particles.v[i][1] + params.xi * particles.v_smoothed[i][1];
        particles.v[i][2] = 0.0;
        dx[0] = params.dt * ((1-params.xi) * particles.v[i][0] + params.xi * particles.v_smoothed[i][0]);
        dx[1] = params.dt * ((1-params.xi) * particles.v[i][1] + params.xi * particles.v_smoothed[i][1]);
      } else {
        particles.v[i][0] = (1-params.xi) * particles.v[i][0] + params.xi * particles.v_smoothed[i][0];
        particles.v[i][1] = (1-params.xi) * particles.v[i][1] + params.xi * particles.v_smoothed[i][1];
        particles.v[i][0] += params.dt * particles.a[i][0];
        particles.v[i][1] += params.dt * particles.a[i][1];
        particles.v[i][2] = 0.0;
        dx[0] = params.dt * particles.v[i][0];
        dx[1] = params.dt * particles.v[i][1];
      }
      particles.T[i]+= params.dt*particles.dt[i]*0.001;
      if(particles.T[i]>Tmax)
       Tmax = particles.T[i];
      if(particles.T[i]<Tmin)
       Tmin = particles.T[i];

      //particles.dt[i]*=params.alpha;

      bool tc= testAllCollision(i,h*0.17,dx);
      if(tc==false)
      {

       particles.x[i][0] += dx[0];
       particles.x[i][1] += dx[1];
       particles.x[i][2] = 0.0;
      }




      // clear forces
      particles.a[i] = glm::vec4(0.0);

       float tmpNormV=glm::l2Norm(glm::vec3(particles.v[i][0],particles.v[i][1],particles.v[i][2]));
      if(tmpNormV>NormMaxV)
      {
       NormMaxV = tmpNormV;
       NormMaxVid = i;
      }
      if(tmpNormV<NormMinV)
       NormMinV = tmpNormV;
      if(NormMaxV>Vmax)
          Vmax = NormMaxV;

      // enforce boundaries (reflect at boundary, with restitution a)
      // 2D only!
      if (particles.x[i][0] > 2.7) {
        float penetration = (particles.x[i][0] - 2.7);
        particles.x[i][0] = (2.7 - params.a * penetration - 0.001*rand01());
        particles.x[i][1] = ox[1] + (1 + (params.a-1) * penetration/fabs(dx[0])) * params.dt * particles.v[i][1];
        
        particles.v[i][0] *= -params.a;
        particles.v[i][1] *= params.a;
      } else if (particles.x[i][0] < -0.7) {
        float penetration = -particles.x[i][0]-0.7;
        particles.x[i][0] = -0.7+params.a * penetration + 0.001*rand01();
        particles.x[i][1] = ox[1] + (1 + (params.a-1) * penetration/fabs(dx[0])) * params.dt * particles.v[i][1];
        
        particles.v[i][0] *= -params.a;
        particles.v[i][1] *= params.a;
      }
      
      if (particles.x[i][1] < 0.05) {
        float penetration = 0.05-particles.x[i][1];
        particles.x[i][0] = ox[0] + (1 + (params.a-1) * penetration/fabs(dx[1])) * params.dt * particles.v[i][0];
        particles.x[i][1] = 0.05+params.a * penetration + 0.001*rand01();
        
        particles.v[i][0] *= params.a;
        particles.v[i][1] *= -params.a;
      }

    }

    time+= params.dt;
 }


    std::chrono::duration<double> diff1= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-t1);
    tcollision +=diff1;
    //std:: cout << "Tmin = " << Tmin << " Tmax = " << Tmax << std::endl;
    n_step++;
    steps++;
    if(n_step % 500 == 0)
    {
        m_neighborhoodSearch->z_sort();
        for (auto j = 0u; j < m_neighborhoodSearch->n_point_sets(); ++j)
        {
            auto const& d = m_neighborhoodSearch->point_set(j);
            d.sort_field(particles.x.data());
            d.sort_field(particles.v.data());
            d.sort_field(particles.C.data());
        }

    }
  }
  
  inline bool testAllCollision(int pId,float r,std::array<float, 3> ndx)
  {
      unsigned int s = polygonIndex.size();
      if(s>1)
      {
          s--;
       bool resp = false;
       unsigned int i;
               for(i=0;i<s;i++)
               {

                  if (testCollision(i,pId,r,ndx))
                   return true;
               }
               return resp;
      }
      else
      return false;
  }

  inline bool testCollision(unsigned int i,int pId,float r,std::array<float, 3> &ndx)
  {
      float a =polygonVertex[i].y-polygonVertex[i+1].y;
      float b = polygonVertex[i+1].x-polygonVertex[i].x;
      float c = -b*polygonVertex[i].y-a*polygonVertex[i].x;
      glm::vec3 ns(a,b,0);
      float den = glm::l2Norm(glm::vec3(ns.x,ns.y,ns.z));
      glm::vec3 v(particles.v[pId][0],particles.v[pId][1],particles.v[pId][2]);
      glm::vec3 vn(particles.v[pId][0],particles.v[pId][1],particles.v[pId][2]);
      glm::normalize(vn);
      glm::normalize(ns);
      float vpn =glm::dot(v,ns);
      if(vpn<0)
      {

          float d =glm::abs(a*particles.x[pId][0]+b*particles.x[pId][1]+c)/den;
          if(d<r)
          {
              if(glm::abs(b)>glm::abs(a))
              {
              float t = (particles.x[pId][0]-polygonVertex[i][0])/b;
              if((t>0.0)&&(t<1.0))
              {
                  float co =2.0*(glm::dot(vn,ns));
                  glm::vec3 rv =   vn - co*ns;
                  glm::normalize(rv);
                  particles.v[pId][0] = glm::l2Norm(glm::vec3(v.x,v.y,v.z))*(params.a)*rv[0];
                  particles.v[pId][1] = glm::l2Norm(glm::vec3(v.x,v.y,v.z))*(params.a)*rv[1];

                  ndx[0] = params.dt*particles.v[pId][0];
                  ndx[1] = params.dt*particles.v[pId][1];

                   particles.x[pId][0] +=  ndx[0];
                   particles.x[pId][1] +=  ndx[1];
                   particles.x[pId][2] =  0.0;
                  //pi.v
                      return true;
              }
              }
              else
              {
                  float t = -(particles.x[pId][1]-polygonVertex[i].y)/a;
                  if((t>0.0)&&(t<1.0))
                  {
                      float co =2.0*(glm::dot(vn,ns));
                      glm::vec3 rv =   vn - co*ns;
                      glm::normalize(rv);
                      particles.v[pId][0] = glm::l2Norm(glm::vec3(v.x,v.y,v.z))*(params.a)*rv[0];
                      particles.v[pId][1] = glm::l2Norm(glm::vec3(v.x,v.y,v.z))*(params.a)*rv[1];


                      ndx[0] = params.dt*particles.v[pId][0];
                      ndx[1] = params.dt*particles.v[pId][1];

                       particles.x[pId][0] +=  ndx[0];
                       particles.x[pId][1] +=  ndx[1];
                       particles.x[pId][2] =  0.0;
                      //pi.v
                          return true;
                  }
              }
          }
      }
      return false;
  }
inline void WriteScreenImage() {

	/*
	 * GET FROM http://local.wasp.uwa.edu.au/~pbourke/rendering/windowdump/
	 * 
	 Write the current view to a file
	 The multiple fputc()s can be replaced with
	 fwrite(image,width*height*3,1,fptr);
	 If the memory pixel order is the same as the destination file format.
	 */

	int i, j;
	FILE *fptr;
	char fname[32];
	unsigned char *image;


	/* Allocate our buffer for the image */
    /* image = reinterpret_cast<unsigned char*>(malloc(3*width*height*sizeof(char)));
	if (image == NULL) {
		fprintf(stderr, "Failed to allocate memory for image\n");
		//return (false);
	}

	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	sprintf(fname, "%s_%04d.ppm", this->screenFilename.c_str(),
			this->screenFilenameNumber);

	if ((fptr = fopen(fname, "w")) == NULL) {
		fprintf(stderr, "Failed to open file for window dump\n");
//		return false;
    }*/

	/* Copy the image into our buffer */
    /*glReadBuffer(GL_BACK_LEFT);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);*/

	/* Write the raw file */
    /*fprintf(fptr, "P6\n%d %d\n255\n", width, height);
	for (j = height - 1; j >= 0; j--) {
		for (i = 0; i < width; i++) {
			fputc(image[3 * j * width + 3 * i + 0], fptr);
			fputc(image[3 * j * width + 3 * i + 1], fptr);
			fputc(image[3 * j * width + 3 * i + 2], fptr);
		}
	}
	fclose(fptr);
    char convert[256];
    char dele[256];
    sprintf(convert,"convert %s %s_%04d.png ", fname,this->screenFilename.c_str(),this->screenFilenameNumber);
    system(convert);
    sprintf(dele,"rm %s ", fname);
    system(dele);*/
/* Clean up */
	/* Clean up */
    sprintf(fname, "%s_%04d.png", this->screenFilename.c_str(),
            this->screenFilenameNumber);

    GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        int x = viewport[0];
        int y = viewport[1];
        int width = viewport[2];
        int height = viewport[3];

        char *data = (char*) malloc((size_t) (width * height * 3)); // 3 components (R, G, B)


        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_flip_vertically_on_write(1);
        int saved = stbi_write_png(fname, width, height, 3, data, 0);

        free(data);

	screenFilenameNumber++;
	// free(image);
	//return true;
}


  inline float smoothing_radius() {
    return h;
  }
  
  // returns the approximate number of neighbors
  inline int smoothing_radius(float newh) {
    h = newh;
    pkernel = pressure_kernel(h);
    vkernel = viscosity_kernel(h);
    pkernelAvx = pressure_kernel_avx(h);
    vkernelAvx = viscosity_kernel_avx(h);
    //grid.queryRadius(h);
    //tree.queryRadius(h);
    h_squared = Scalarf8(0.01*h*h);
    return compute_mass();
  }
  
  // returns the approximate number of neighbors
  inline int compute_mass() {
    // set mass s.t. sampling approximately yields density rho0 
    std::vector<glm::vec3> pos = sample_hex(spacing, 0, Vector(-h, -h, 0), Vector(h, h, 0));
    float rho_a = 0;
    int q = 0;
    std::cout << "pos size = " << pos.size()  << " spacing = " << spacing << std::endl;
    for (int i = 0; i < pos.size(); ++i) {
      //std::cout << "pos["<<i<<"].x =  "<< pos[i].x << " pos["<<i<<"].y =  "<< pos[i].y << " pos["<<i<<"].z =  "<< pos[i].z << std::endl;
      float d = glm::l2Norm(pos[i]);
      float k = pkernel.value(d, d*d);
      rho_a += k;
      if (k > 0)
        q++;
    }
    // normalize mass according to particle density
    m = params.rho0/rho_a;    
    
    m_neighborhoodSearch = new NeighborhoodSearch(h);
    m_neighborhoodSearch->add_point_set(particles.x.front().data(), particles.m_count, true, true);
    /*m_neighborhoodSearch->add_point_set(Points_positions.front().data(), Points_positions.size(), true, true);
        m_neighborhoodSearch->find_neighbors();

        m_neighborhoodSearch->update_point_sets();
        std::vector<std::vector<unsigned int>> neighbors2;
        m_neighborhoodSearch->find_neighbors(0, 1, neighbors2);
        std::vector<std::vector<unsigned int>> neighbors3;
        m_neighborhoodSearch->find_neighbors(1, 2, neighbors3);*/

    m_neighborhoodSearch->z_sort();
        for (auto j = 0u; j < m_neighborhoodSearch->n_point_sets(); ++j)
        {
            auto const& d = m_neighborhoodSearch->point_set(j);
            d.sort_field(particles.x.data());
            d.sort_field(particles.C.data());

        }
        m_neighborhoodSearch->find_neighbors();

    std::cout << "rho0 = " << params.rho0 << ", h = " << h << ", q = " << q << ", m = " << m << std::endl;
    
    return q;
  }
  
  inline int n_particles() const {
    return particles.m_count;
  }
  
  //inline Particle const &particle(int i) const {
    //return particles[i];
  //}

  //inline Particle &particle(int i) {
    //return particles[i];
  //}
  


  //inline HashGrid const &hashgrid() const {
    //return grid;
  //}
  
  // surface extraction
  inline void surfaceCells(int cs) {
    xsurfacecells = cs;
    ysurfacecells = cs * 1.1 + 1;
    cellsize = 3.4f/cs;
  }

inline void generateGrid()
{
    int i,nl= 2.0/cellsize;
    //gridVertices = new float[nl*4*3];
    for (i=0;i<nl;i++)
    {
        gridVertices[i*6]=-1.0+cellsize*i;
        gridVertices[i*6+1]=-1.0;
        gridVertices[i*6+2]=0.0;
        gridVertices[i*6+3]=-1.0+cellsize*i;
        gridVertices[i*6+4]=1.0;
        gridVertices[i*6+5]=0.0;
        gridVertices[6*nl+i*6]=-1.0;
        gridVertices[6*nl+i*6+1]=-1.0+cellsize*i;
        gridVertices[6*nl+i*6+2]=0.0;
        gridVertices[6*nl+i*6+3]=1.0;
        gridVertices[6*nl+i*6+4]=-1.0+cellsize*i;
        gridVertices[6*nl+i*6+5]=0.0;
    }
    int s= nl*4;
    //gridIndices=nullptr;
   // gridIndices = (unsigned int*) new unsigned int[s];
    for (i=0;i<nl;i++)
    {
        gridIndices[i*2]=i*2;
        gridIndices[i*2+1]=i*2+1;
        gridIndices[2*nl+i*2]=2*nl+i*2;
        gridIndices[2*nl+i*2+1]=2*nl+i*2+1;
    }
}
  
  inline void drawSurfaceGrid() const {
    glLineWidth(1);
    glColor3f(0.3, 0.3, 0.3);
    glBegin(GL_LINES);
    for (float x = 0; x <= 2.0; x += cellsize) {
      glVertex2f(x, 0);
      glVertex2f(x, 2.0);
    }
    for (float y = 0; y <= 2.0; y += cellsize) {
      glVertex2f(0, y);
      glVertex2f(2.0, y);
    }
    glEnd();    
  }
  
  inline void drawSurface()  {
      curveVertex.clear();
      curveIndex.clear();
    // Do marching squares, iterate over all cells. 
    // If the fluid is "sparse", ie if there are lots of cells and particles in only few of them,
    // it makes sense to do fast marching stating at each particle (but only once per cell). 
    std::vector<float> value((xsurfacecells+1) * (ysurfacecells+1));
    NeighborData nbs;
    #pragma omp parallel for collapse(2)
    for (int x = 0; x <= xsurfacecells; ++x) {
      for (int y = 0; y <= ysurfacecells; ++y) {
        glm::vec3 p(-0.7+x*cellsize, y*cellsize, 0);
        float  px[3];
        px[0]=p.x;px[1]=p.y;px[2]=p.z;
        float &rho = value[y * (xsurfacecells+1) + x];
        //tree.neighbors(p, nbs);
        std::vector<std::vector<unsigned int>> v_neighbors;
        m_neighborhoodSearch->find_neighbors(px,v_neighbors);
        // compute density at each point
        rho = 0;
        for (int i = 0; i < v_neighbors[0].size(); ++i) {

          float d = glm::l2Norm((p-glm::vec3(particles.x[v_neighbors[0][i]][0],particles.x[v_neighbors[0][i]][1],particles.x[v_neighbors[0][i]][2])));
          float d_squared =d*d;
          rho += m * pkernel.value(d,d_squared);
        }
        rho*=0.0025;
      }   
    } 

    /* 
    // visualize grid values
    glPointSize(3);
    glBegin(GL_POINTS);
    for (int x = 0; x <= xsurfacecells; ++x) {
      for (int y = 0; y <= ysurfacecells; ++y) {    
        // get the four corner values
        // order is x y
        float v00 = value[y * (xsurfacecells+1) + x] - isovalue;
        if (v00 >= 0)
          glColor3f(1, 0, 0);
        else
          glColor3f(0, 1, 0);
            
        glVertex2f(x*cellsize, y*cellsize);
      }
    }        
    glEnd();
    */
    
    unsigned int index=0;
    //#pragma omp parallel for collapse(2)
    for (int x = 0; x < xsurfacecells; ++x) {
      for (int y = 0; y < ysurfacecells; ++y) {    
        // get the four corner values
        // order is x y
        
        // v10 - yhi - v11
        //  |           |
        // xlo         xhi
        //  |           |
        // v00 - ylo - v10

        struct intersector {
          // coordinates
          float xlo, xhi, ylo, yhi;
          // values
          float v00, v10, v01, v11;
          
          // find zero crossing by interpolation
          inline float zero(float v0, float v1, float xlo, float xhi) const {
            float a = v0/(v0-v1);     
            return (1-a) * xlo + a * xhi;
          }
          
          // get marching squares code
          inline char code() const {
            return ((v00 < 0) * 1) | ((v10 < 0) * 2) | ((v01 < 0) * 4) | ((v11 < 0) * 8);            
          }
          
          // get crossing vertices
          inline void vxlo(std::vector<glm::vec3> &v) const {
            if (v00 * v01 > 0) throw;
            v.push_back(glm::vec3(xlo, zero(v00, v01, ylo, yhi),0.0));
          }
          inline void vxhi(std::vector<glm::vec3> &v) const {
            if (v10 * v11 > 0) throw;
            v.push_back(glm::vec3(xhi, zero(v10, v11, ylo, yhi),0.0));
          }          
          inline void vylo(std::vector<glm::vec3> &v) const {
            if (v00 * v10 > 0) throw;
            v.push_back(glm::vec3(zero(v00, v10, xlo, xhi), ylo,0.0));
          }
          inline void vyhi(std::vector<glm::vec3> &v) const {
            if (v01 * v11 > 0) throw;
            v.push_back(glm::vec3(zero(v01, v11, xlo, xhi), yhi,0.0));
          }
        };
        
        intersector intersect;
        
        intersect.v00 = value[y * (xsurfacecells+1) + x] - isovalue;
        intersect.v10 = value[y * (xsurfacecells+1) + x+1] - isovalue;
        intersect.v01 = value[(y+1) * (xsurfacecells+1) + x] - isovalue;
        intersect.v11 = value[(y+1) * (xsurfacecells+1) + x+1] - isovalue;

        intersect.xlo = -0.7+ x * cellsize;
        intersect.xhi = intersect.xlo + cellsize;
        intersect.ylo = y * cellsize;
        intersect.yhi = intersect.ylo + cellsize;
        
        // marching squares, draw directly
        // code(binary) = v11 v01 v10 v00
    
        switch (intersect.code()) {
          case 0:
          case 0xf:
            // no intersections
            break;
          case 1: // 0001
          case 0xe: // 1110
            intersect.vxlo(curveVertex);
            curveIndex.push_back(index);index++;
            intersect.vylo(curveVertex);
            curveIndex.push_back(index);index++;

            break;
          case 2: // 0010
          case 0xd: // 1101
            intersect.vxhi(curveVertex);
            curveIndex.push_back(index);index++;
            intersect.vylo(curveVertex);
            curveIndex.push_back(index);index++;

            break;
          case 3: // 0011
          case 0xc: // 1100
            intersect.vxlo(curveVertex);
            curveIndex.push_back(index);index++;
            intersect.vxhi(curveVertex);
            curveIndex.push_back(index);index++;

            break;
          case 4: // 0100
          case 0xb: // 1011
            intersect.vxlo(curveVertex);
            curveIndex.push_back(index);index++;
            intersect.vyhi(curveVertex);
            curveIndex.push_back(index);index++;

            break;
          case 5: // 0101
          case 0xa: // 1010
            intersect.vyhi(curveVertex);
            curveIndex.push_back(index);index++;
            intersect.vylo(curveVertex);
            curveIndex.push_back(index);index++;

            break;
          case 6: // 0110
          case 9: // 1001
            intersect.vxlo(curveVertex);
            curveIndex.push_back(index);index++;
            intersect.vylo(curveVertex);
            curveIndex.push_back(index);index++;
            intersect.vxhi(curveVertex);
            curveIndex.push_back(index);index++;
            intersect.vyhi(curveVertex);
            curveIndex.push_back(index);index++;

            break;
          case 7: // 0111
          case 8: // 1000
            // intersections on yhi and xhi
            intersect.vxhi(curveVertex);
            curveIndex.push_back(index);index++;
            intersect.vyhi(curveVertex);
            curveIndex.push_back(index);index++;

            break;
        }
      }
    }

  }
};


#endif
