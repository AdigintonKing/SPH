#ifdef __APPLE__
#include <GLUT/glut.h>
#else
//#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <iostream>
#include "sph.hh"

int _fpsCount = 0;
//float fps = 0; // this will store the final fps for the last second
double lastTime,lt;
int nbFrames = 0;
SPH sph;
int niter=0;
CGAL::Timer task_timer_over_all;
unsigned int rendermode=2;
bool running = false, drawgrid = false, drawsurface = false, drawhash = false, drawtree = false, saveImageFile=false,spheresok=true, polygonok=false;
bool firstrender=true;
bool firstVFrender=true;
bool firstVFrenderColor=true;
GLuint m_textureMap;
float const *color_map = nullptr;

int test = int(sph.params.frame_dt / sph.params.dt);

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

const char *vertexGridShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentGridShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(0.7f, 0.7f, 0.7f, 1.0f);\n"
    "}\n\0";

const char *vertexCurveShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 view;\n"
    "uniform mat4 model;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model *vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentCurveShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(0.0f, 1.0f, 1.0f, 1.0f);\n"
    "}\n\0";


const char *vertexPolygonShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 view;\n"
    "uniform mat4 model;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model *vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentPolygonShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
    "}\n\0";


unsigned int VBO, VAO, EBO,gridVBO, gridVAO, gridEBO,curveVBO=0, curveVAO, curveEBO,polygonVBO=0, polygonVAO, polygonEBO,particlesVBO=0, particlesVAO, particlesScalarVBO, particlesVelocitiesVBO, particlesEBO,vectorFieldVAO=0,vectorFieldVBO,vectorFieldEBO,vectorFieldScalarVBO, vectorFieldVelocitiesVBO,fbo, textColorbuffer;
unsigned int vectorFieldColorVAO=0,vectorFieldColorVBO,vectorFieldColorEBO,vectorFieldColorScalarVBO, vectorFieldColorVelocitiesVBO;
int vertexShader,fragmentShader,shaderProgram,vertexGridShader,fragmentGridShader,shaderGridProgram,vertexCurveShader,fragmentCurveShader,shaderCurveProgram,vertexPolygonShader,fragmentPolygonShader,shaderPolygonProgram;

float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

void CalculateFrameRate()
{
    static float framesPerSecond = 0.0f;
    static int fps;
    static float lastTime = 0.0f;
    float currentTime ;//= GetTickCount() * 0.001f;
    ++framesPerSecond;
    //glPrint("Current Frames Per Second: %d\n\n", fps);
    if (currentTime - lastTime > 1.0f)
    {
        //lastTime = currentTime;
        fps = (int)framesPerSecond;
        framesPerSecond = 0;
    }
}

void compileVertexShaders()
{
    // build and compile our shader program
       // ------------------------------------
       // vertex shader
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
       glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
       glCompileShader(vertexShader);
       // check for shader compile errors
       int success;
       char infoLog[512];
       glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
       if (!success)
       {
           glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
           std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
       }
       // fragment shader
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
       glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
       glCompileShader(fragmentShader);
       // check for shader compile errors
       glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
       if (!success)
       {
           glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
           std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
       }
       // link shaders
        shaderProgram = glCreateProgram();
       glAttachShader(shaderProgram, vertexShader);
       glAttachShader(shaderProgram, fragmentShader);
       glLinkProgram(shaderProgram);
       // check for linking errors
       glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
       if (!success) {
           glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
           std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
       }
       glDeleteShader(vertexShader);
       glDeleteShader(fragmentShader);

       // set up vertex data (and buffer(s)) and configure vertex attributes
       // ------------------------------------------------------------------
       float vertices[] = {
            -0.99f,  1.0f, 0.0f,  // top right
            -0.99f, -0.93f, 0.0f,  // bottom right
            0.99f, -0.93f, 0.0f,  // bottom left
           0.99f,  1.0f, 0.0f   // top left
       };
       unsigned int indices[] = {  // note that we start from 0!
           0, 1, 2,3,  // first Triangle

       };

       glGenVertexArrays(1, &VAO);
       glGenBuffers(1, &VBO);
       glGenBuffers(1, &EBO);
       // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
       glBindVertexArray(VAO);

       glBindBuffer(GL_ARRAY_BUFFER, VBO);
       glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

       glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
       glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

       glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
       glEnableVertexAttribArray(0);

       // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
       glBindBuffer(GL_ARRAY_BUFFER, 0);

       // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
       //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

       // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
       // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
       glBindVertexArray(0);
};

void compileCurveShaders()
{

        // build and compile our shader program
           // ------------------------------------
           // vertex shader
            vertexCurveShader = glCreateShader(GL_VERTEX_SHADER);
           glShaderSource(vertexCurveShader, 1, &vertexCurveShaderSource, NULL);
           glCompileShader(vertexCurveShader);
           // check for shader compile errors
           int success;
           char infoLog[512];
           glGetShaderiv(vertexCurveShader, GL_COMPILE_STATUS, &success);
           if (!success)
           {
               glGetShaderInfoLog(vertexCurveShader, 512, NULL, infoLog);
               std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
           }
           // fragment shader
            fragmentCurveShader = glCreateShader(GL_FRAGMENT_SHADER);
           glShaderSource(fragmentCurveShader, 1, &fragmentCurveShaderSource, NULL);
           glCompileShader(fragmentCurveShader);
           // check for shader compile errors
           glGetShaderiv(fragmentCurveShader, GL_COMPILE_STATUS, &success);
           if (!success)
           {
               glGetShaderInfoLog(fragmentCurveShader, 512, NULL, infoLog);
               std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
           }
           // link shaders
            shaderCurveProgram = glCreateProgram();
           glAttachShader(shaderCurveProgram, vertexCurveShader);
           glAttachShader(shaderCurveProgram, fragmentCurveShader);
           glLinkProgram(shaderCurveProgram);
           // check for linking errors
           glGetProgramiv(shaderCurveProgram, GL_LINK_STATUS, &success);
           if (!success) {
               glGetProgramInfoLog(shaderCurveProgram, 512, NULL, infoLog);
               std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
           }
           glDeleteShader(vertexCurveShader);
           glDeleteShader(fragmentCurveShader);

}

void compilePolygonShaders()
{

        // build and compile our shader program
           // ------------------------------------
           // vertex shader
            vertexPolygonShader = glCreateShader(GL_VERTEX_SHADER);
           glShaderSource(vertexPolygonShader, 1, &vertexPolygonShaderSource, NULL);
           glCompileShader(vertexPolygonShader);
           // check for shader compile errors
           int success;
           char infoLog[512];
           glGetShaderiv(vertexPolygonShader, GL_COMPILE_STATUS, &success);
           if (!success)
           {
               glGetShaderInfoLog(vertexPolygonShader, 512, NULL, infoLog);
               std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
           }
           // fragment shader
            fragmentPolygonShader = glCreateShader(GL_FRAGMENT_SHADER);
           glShaderSource(fragmentPolygonShader, 1, &fragmentPolygonShaderSource, NULL);
           glCompileShader(fragmentPolygonShader);
           // check for shader compile errors
           glGetShaderiv(fragmentPolygonShader, GL_COMPILE_STATUS, &success);
           if (!success)
           {
               glGetShaderInfoLog(fragmentPolygonShader, 512, NULL, infoLog);
               std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
           }
           // link shaders
            shaderPolygonProgram = glCreateProgram();
           glAttachShader(shaderPolygonProgram, vertexPolygonShader);
           glAttachShader(shaderPolygonProgram, fragmentPolygonShader);
           glLinkProgram(shaderPolygonProgram);
           // check for linking errors
           glGetProgramiv(shaderPolygonProgram, GL_LINK_STATUS, &success);
           if (!success) {
               glGetProgramInfoLog(shaderPolygonProgram, 512, NULL, infoLog);
               std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
           }
           glDeleteShader(vertexPolygonShader);
           glDeleteShader(fragmentPolygonShader);

}



void compileGridShaders()
{
    // build and compile our shader program
       // ------------------------------------
       // vertex shader
        vertexGridShader = glCreateShader(GL_VERTEX_SHADER);
       glShaderSource(vertexGridShader, 1, &vertexGridShaderSource, NULL);
       glCompileShader(vertexGridShader);
       // check for shader compile errors
       int success;
       char infoLog[512];
       glGetShaderiv(vertexGridShader, GL_COMPILE_STATUS, &success);
       if (!success)
       {
           glGetShaderInfoLog(vertexGridShader, 512, NULL, infoLog);
           std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
       }
       // fragment shader
        fragmentGridShader = glCreateShader(GL_FRAGMENT_SHADER);
       glShaderSource(fragmentGridShader, 1, &fragmentGridShaderSource, NULL);
       glCompileShader(fragmentGridShader);
       // check for shader compile errors
       glGetShaderiv(fragmentGridShader, GL_COMPILE_STATUS, &success);
       if (!success)
       {
           glGetShaderInfoLog(fragmentGridShader, 512, NULL, infoLog);
           std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
       }
       // link shaders
        shaderGridProgram = glCreateProgram();
       glAttachShader(shaderGridProgram, vertexGridShader);
       glAttachShader(shaderGridProgram, fragmentGridShader);
       glLinkProgram(shaderGridProgram);
       // check for linking errors
       glGetProgramiv(shaderGridProgram, GL_LINK_STATUS, &success);
       if (!success) {
           glGetProgramInfoLog(shaderGridProgram, 512, NULL, infoLog);
           std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
       }
       glDeleteShader(vertexGridShader);
       glDeleteShader(fragmentGridShader);


       glGenVertexArrays(1, &gridVAO);
       glGenBuffers(1, &gridVBO);
       glGenBuffers(1, &gridEBO);
       // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
       glBindVertexArray(gridVAO);

       glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
       glBufferData(GL_ARRAY_BUFFER, sizeof(sph.gridVertices), sph.gridVertices, GL_STATIC_DRAW);

       glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridEBO);
       glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sph.gridIndices), sph.gridIndices, GL_STATIC_DRAW);

       glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
       glEnableVertexAttribArray(0);

       // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
       glBindBuffer(GL_ARRAY_BUFFER, 0);

       // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
       //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

       // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
       // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
       glBindVertexArray(0);
};

using namespace std;
Texture2D::Texture2D()
    : Width(0), Height(0), Internal_Format(GL_RGB), Image_Format(GL_RGB), Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT), Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR)
{

}

bool Texture2D::load_texture(const char *file_name)
{
    int x, y, n;
    int force_channels        = 4;
    unsigned char* image_data = stbi_load( file_name, &x, &y, &n, force_channels );
    if ( !image_data ) {
      fprintf( stderr, "ERROR: could not load %s\n", file_name );
      return false;
    }
    // NPOT check
    if ( ( x & ( x - 1 ) ) != 0 || ( y & ( y - 1 ) ) != 0 ) { fprintf( stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name ); }
    int width_in_bytes    = x * 4;
    unsigned char* top    = NULL;
    unsigned char* bottom = NULL;
    unsigned char temp    = 0;
    int half_height       = y / 2;

    for ( int row = 0; row < half_height; row++ ) {
      top    = image_data + row * width_in_bytes;
      bottom = image_data + ( y - row - 1 ) * width_in_bytes;
      for ( int col = 0; col < width_in_bytes; col++ ) {
        temp    = *top;
        *top    = *bottom;
        *bottom = temp;
        top++;
        bottom++;
      }
    }
    glGenTextures( 1, &(this->ID) );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, ID );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data );
    glGenerateMipmap( GL_TEXTURE_2D );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    GLfloat max_aniso = 0.0f;
    glGetFloatv( 0x84FF, &max_aniso );
    // set the maximum!
    glTexParameterf( GL_TEXTURE_2D, 0x84FE, max_aniso );
    return true;
}

void Texture2D::Generate(unsigned int width, unsigned int height, unsigned char* data)
{
    glGenTextures(1, &this->ID);
    this->Width = width;
    this->Height = height;
    // create Texture
    glBindTexture(GL_TEXTURE_2D, this->ID);
    glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, width, height, 0, this->Image_Format, GL_UNSIGNED_BYTE, data);
    // set Texture wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max);
    // unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::loadTextureFromFile(const char *file, bool alpha)
{
    if (alpha)
    {
        this->Internal_Format = GL_RGBA;
        this->Image_Format = GL_RGBA;
    }
    // load image
    int width, height, nrChannels;
    unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 4);
    // now generate texture
    this->Generate(width, height, data);
    // and finally free image data
    stbi_image_free(data);
}

void Texture2D::Bind() const
{
    glBindTexture(GL_TEXTURE_2D, this->ID);
}
ImTextureID my_tex_id;
struct ViewParticle{
    glm::vec3 pos;
     glm::vec4 Color;

    float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

    bool operator<(const ViewParticle& that) const {
        // Sort in reverse order : far particles drawn first.
        return this->cameradistance > that.cameradistance;
    }
};

class Timer {
protected:
double start, finish;
public:
std::vector<double> times;
Timer() { start = clock();}
void record() {
times.push_back(time());
}
void reset_vectors() {
times.erase(times.begin(), times.end());
}
void restart() { start = clock(); }
void stop() { finish = clock(); }
double time() const { return ((double)(finish - start))/CLOCKS_PER_SEC; }
};


Timer  timer;
int frames=0;
int polygonIndex=0;

//const int MaxParticles = 20000;
ViewParticle *ParticlesContainer;
void SortParticles(const uint np){
    std::sort(&ParticlesContainer[0], &ParticlesContainer[np]);
}
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, int button, int action, int mods);

void hsvToRgb(float h, float s,float v, glm::vec4 &rgb);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const * path);
void renderParticlesInstance();
void renderParticles();
void renderVectorField();
void renderVectorFieldColor();
void renderSphere(glm::vec3 &pos,glm::vec3 &color);
void renderImplicitCurve();
void renderPolygonObstacle();
void initImGUI(GLFWwindow* window);
void renderGUI();
void printMatrix(glm::mat4x4 M);
glm::mat4 projection;
glm::vec3 *translations;
float *scalarField;
glm::vec4 *instanceColors;
/*************/

//glm::mat4 model;
unsigned int instanceVBO=0;
uint instancecolorVBO=0;
unsigned int sphereVAO = 0;
unsigned int indexCount;
unsigned int  quadVAO,quadVBO;
unsigned int  quadVAOScreen,quadVBOScreen;
Texture2D  particleTexture;
//static int ox, oy;
bool buttondown[3] = {false, false, false};
bool vcolorok=false;
bool gridok=false;
bool curveok=false;
bool pok=true;
Real Sc;

GLfloat polygonVertices[] =
   {
       0, 10, 0,
       0, 0, 0,
       2, 0, 0,
       2, 10, 0
   };

float mouse_force = 0.9;
Vector mouse_point(std::numeric_limits<double>::infinity(), 
                   std::numeric_limits<double>::infinity(), 
                   std::numeric_limits<double>::infinity());
std::vector<NeighborData> selected;

struct {
  const char *name;
  Parameters params;
} parameters[] = {
// name          dt    r  gamma K rho0 zeta  nu xi  xsph             g          a 
{"incompressible", {0.001, 2.5, 5, 1000, 998.6, 0.8, 0.002, 0.04, false, glm::vec4(0, -9.81, 0,0), 0.7}},
{"compressible", {0.001, 2.5, 1, 500, 200.6, 0.8, 0.02, 0.4, true, glm::vec4(0, -9.81, 0,0), 0.7}},
{"real XSPH", {0.001, 2.5, 5, 1000, 998.6, 0.8, 0.002, 0.04, true, glm::vec4(0, -9.81, 0,0), 0.7}},
{"fake XSPH high", {0.001, 2.5, 5, 1000, 998.6, 0.8, 0.002, 0.8, false, glm::vec4(0, -9.81, 0,0), 0.7}},
{"viscosity too low", {0.001, 2.5, 5, 1000, 998.6, 0.8, 0.001, 0.04, false, glm::vec4(0, -9.81, 0,0), 0.7}},
{"viscosity ok (low)", {0.001, 2.5, 5, 1000, 998.6, 0.8, 0.003, 0.04, false, glm::vec4(0, -9.81, 0,0), 0.7}},
{"viscosity highest", {0.001, 2.5, 5, 1000, 998.6, 0.8, 0.05, 0.04, false, glm::vec4(0, -9.81, 0,0), 0.7}},
{"viscosity too high", {0.001, 2.5, 5, 1000, 998.6, 0.8, 0.1, 0.04, false, glm::vec4(0, -9.81, 0,0), 0.7}},
{"low smoothing radius", {0.001, 1.8, 5, 1000, 998.6, 0.8, 0.001, 0.04, false, glm::vec4(0, -9.81, 0,0), 0.7}},
{"high smoothing radius", {0.001, 5, 5, 1000, 998.6, 0.8, 0.001, 0.04, false, glm::vec4(0, -9.81, 0,0), 0.7}}
};

void idle()
{

    // render OpenGL here


    if (running) {
    sph.step();
    niter++;


    if (niter % test == 0)
    {


        if(pok)
        {
            glEnable( GL_PROGRAM_POINT_SIZE );
            glPointParameteri( GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT );

            glEnable( GL_BLEND );
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);                glDepthMask( GL_FALSE );
            switch(rendermode)
            {
                case 1:
                {
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    renderParticlesInstance();
                    break;
                }
                case 2:
                {
                    glBlendFunc(GL_SRC_COLOR, GL_SRC_ALPHA);
                    renderParticles();
                    break;
                }
            case 3:
            {
                glBlendFunc(GL_SRC_COLOR, GL_SRC_ALPHA);
                //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                renderVectorFieldColor();
                break;
            }
                case 0:
                {
                    glBlendFunc(GL_SRC_COLOR, GL_SRC_ALPHA);
                    renderVectorField();
                    break;
                }
            }



            glDisable( GL_BLEND );
            glDepthMask( GL_TRUE );
            glDisable( GL_PROGRAM_POINT_SIZE );
        }
        renderGUI();
        glfwSwapBuffers(window);
        if(saveImageFile)
        {
          //sph.surfaceCells(50);
          sph.WriteScreenImage();
        }



   // glutPostRedisplay();
    }
    /*double currentTime = glfwGetTime();
         nbFrames++;
         if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
             // printf and reset timer
             printf("%f ms/frame\n", 1.0*double(nbFrames));
             nbFrames = 0;
             lastTime += 1.0;
         }*/
  }
    else
    {
        //glClearColor(0.0f, 0.3f, 0.5f, 1.0f);
         //glClear( GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
        //renderParticlesInstance();
        //glfwSwapBuffers(window);
    }


   //  glfwPollEvents();
}

glm::vec3 lightPos(0.2f, 0.75f, 2.0f);
glm::vec3 lightPos1(0.8f, 0.75f, 2.0f);

int main(int argc, char ** argv) 
{

     InitializeMagick(*argv);
/*  glutInitWindowSize(sph.winWidth, sph.winHeight);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA | GLUT_DOUBLE);
  glutCreateWindow("SPH");

  sph.init();

  // GL init
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glutDisplayFunc(redraw);
  glutMotionFunc(motion);
  glutPassiveMotionFunc(motion);
  glutMouseFunc(button);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutIdleFunc(idle);
  
  glutMainLoop();
 */

    // glfw: initialize and configure
    // ------------------------------
    sph.initFromCloud();
    //sph.initFromImage();
    //sph.init();
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(sph.winWidth, sph.winHeight, "SPH", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    cameraPos= cameraInitialPos;
    cameraFront = cameraInitialFront;
    cameraUp    =cameraInitialUp;

    camera = new Camera(glm::vec3(1.0f, 1.0f, 2.5f),glm::vec3(0.0f, 1.0f,  0.0f));

    view= camera->calculate_lookAt_matrix(cameraPos,cameraFront,cameraUp);
    projection = glm::perspective(glm::radians(fov), (float)sph.winWidth / (float)sph.winHeight, 0.1f, 100.0f);
    vectorFieldShader.init("../resources/properties/vs_vectorField.glsl","../resources/properties/fs_vectorField.glsl","../resources/properties/gs_vectorField_color.glsl");
    vectorFieldShader.use();
    vectorFieldShader.setFloat("radius",sph.h*0.5);
    vectorFieldShader.setMat4("projection_matrix",projection);
    vectorFieldShader.setMat4("modelview_matrix",view);
    vectorFieldShader.setBool("vok",false);
    vectorFieldShaderColor.init("../resources/properties/vs_vectorField.glsl","../resources/properties/fs_vectorField_color.glsl","../resources/properties/gs_vectorField_color.glsl");
    glm::vec2 Res(sph.winWidth,sph.winHeight);
    vectorFieldShaderColor.use();
    vectorFieldShaderColor.setVec2("iResolution",Res);
    vectorFieldShaderColor.setFloat("radius",sph.h*0.5);
    vectorFieldShaderColor.setMat4("projection_matrix",projection);
    vectorFieldShaderColor.setMat4("modelview_matrix",view);
    vectorFieldShaderColor.setBool("vok",false);

    m_shader_scalar.init("../resources/properties/vs_points_scalar.glsl","../resources/properties/fs_VF_test.glsl");
    m_shader_scalar.use();
    m_shader_scalar.setFloat("radius",sph.h*0.31);
    m_shader_scalar.setMat4("projection_matrix",projection);
    m_shader_scalar.setMat4("modelview_matrix",view);
    m_shader_scalar.setBool("vok",false);
    m_shader_scalar.setBool("imageok",imgok);
    //m_shader_scalar.setBool("imageok",true);
    //std::cout << view[0][0] << std::endl;
    //printMatrix(view);
    //printMatrix(glm::transpose(glm::inverse(view)));
    //printMatrix(projection);

    sphereShader.init("../resources/properties/sphere.vs", "../resources/properties/sphere.fs");

    sphereShader.use();
    sphereShader.setFloat("ao", 1.0f);
    sphereShader.setVec3("lightPosition",lightPos);
    sphereShader.setVec3("lightPosition1",lightPos1);
    sphereShader.setFloat("roughness",0.05f);
    //Interactor->sphereShader.setVec3("albedo", 0.0f, 0.0f, 0.6f);
    sphereShader.setVec3("camPos", camera->Position);
    sphereShader.setVec3("lightColor",glm::vec3(1.0f, 1.0f, 1.0f));
    instanceShader.init("../resources/properties/particleInstancing.vs", "../resources/properties/particleInstancing.fs");
    instanceShader.use();
    instanceShader.setInt("sprite",0);
    instanceShader.setFloat("radius",sph.h*0.4);
    instanceShader.setVec3("lightPosition",lightPos);
    instanceShader.setVec3("lightPosition1",lightPos1);

    //projection = glm::ortho(0.0f, (float)sph.winWidth, 0.0f, (float)sph.winHeight);
     instanceShader.setMat4("projection", projection);

     instanceShader.setMat4("view",  view);
     model = glm::mat4(1.0f);
     instanceShader.setMat4("model",model);
     instanceShader.setVec3("camPos",  camera->Position);
    Sc=0.25*sph.h;
    sph.generateGrid();
    compileVertexShaders();
    compileGridShaders();
    compileCurveShaders();
    compilePolygonShaders();
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
     //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ); // polygon drawing mode (GL_POINT, GL_LINE, GL_FILL)
    initImGUI(window);
     double fps;
     char title_string[50];
    // render loop
    // -----------
     glEnable( GL_CULL_FACE );  // cull face
     glCullFace( GL_BACK );     // cull back face
     glFrontFace( GL_CCW );     // GL_CCW for counter clock-wise
     glDepthFunc( GL_LESS );    // depth-testing interprets a smaller value as "closer"
     glEnable( GL_DEPTH_TEST ); // enable depth-testing
     glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

     glBindFramebuffer(GL_FRAMEBUFFER, fbo);
     glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
     //glad_glClearDepth(0.0);
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    lt = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //glClearColor(0.17f, 0.152f, 0.129f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
        // input
        // -----
        processInput(window);



         renderPolygonObstacle();
        // render
        // ------
        glEnable(GL_LINE_WIDTH);
        glLineWidth(2);
        // draw our first triangle
                glUseProgram(shaderProgram);
                glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
                //glDrawArrays(GL_TRIANGLES, 0, 6);
                glDrawElements(GL_LINE_STRIP, 4, GL_UNSIGNED_INT, 0);
                // glBindVertexArray(0); // no need to unbind it every time
        glDisable(GL_LINE_WIDTH);



        if(gridok)
        {
            glEnable(GL_LINE_WIDTH);
            glLineWidth(1);
            // draw our first triangle
                    glUseProgram(shaderGridProgram);
                    glBindVertexArray(gridVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
                    //glDrawArrays(GL_TRIANGLES, 0, 6);
                    glDrawElements(GL_LINES, 316, GL_UNSIGNED_INT, 0);
                    // glBindVertexArray(0); // no need to unbind it every time
            glDisable(GL_LINE_WIDTH);
        }
        if(curveok)
            renderImplicitCurve();


        idle();



        if(!running)
        {
            if(pok)
            {
                glEnable( GL_PROGRAM_POINT_SIZE );
                glPointParameteri( GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT );
                glEnable( GL_BLEND );


                //glDepthMask( GL_FALSE );
                switch (rendermode)
                {
                    case 1:
                    {
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        renderParticlesInstance();
                        break;
                    }
                    case 2:
                    {
                        glBlendFunc(GL_SRC_COLOR, GL_SRC_ALPHA);
                        renderParticles();
                        break;
                    }
                case 3:
                {
                    glBlendFunc(GL_SRC_COLOR, GL_SRC_ALPHA);
                    renderVectorFieldColor();
                    break;
                }
                    case 0:
                    {
                        glBlendFunc(GL_SRC_COLOR, GL_SRC_ALPHA);
                        renderVectorField();
                        break;
                    }
                }


                glDisable( GL_BLEND );
                glDepthMask( GL_TRUE );
                glDisable( GL_PROGRAM_POINT_SIZE );
            }
            renderGUI();
            glfwSwapBuffers(window);
        }
        else
        {
            double CurrentTime = glfwGetTime();
            double past = CurrentTime - lt;
            if(past>0.1) {

            fps = (float)frames/past;
            sprintf(title_string, "Visualizador - FPS = %.2f ", fps);
            glfwSetWindowTitle(window, title_string);
            lt+=past;
            frames = 0;
            }


            frames ++;
        }




          // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------

        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    if(instanceVBO!=0)
    {
        delete [] translations;
        delete [] instanceColors;
        delete [] ParticlesContainer;
    }
    return 0;
 // return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


void hsvToRgb(float h, float s, float v, glm::vec4 &rgb)
{
    int i = (int)floor(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6)
    {
    case 0: rgb[0] = v, rgb[1] = t, rgb[2] = p; break;
    case 1: rgb[0] = q, rgb[1] = v, rgb[2] = p; break;
    case 2: rgb[0] = p, rgb[1] = v, rgb[2] = t; break;
    case 3: rgb[0] = p, rgb[1] = q, rgb[2] = v; break;
    case 4: rgb[0] = t, rgb[1] = p, rgb[2] = v; break;
    case 5: rgb[0] = v, rgb[1] = p, rgb[2] = q; break;
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    int set;
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        float ms;
            case GLFW_KEY_ESCAPE:
                ms = (float) (sph.tsum.count()/sph.n_step);
                std::cout << "Average Neighborhood search took " << ms << " ms" << std::endl;
                std::cout << "Average Pressure computation took " << (float)(sph.tpressure.count()/sph.n_step) << " ms" << std::endl;
                std::cout << "Average Force computation took " << (float) (sph.tforce.count()/sph.n_step) << " ms" << std::endl;
                std::cout << "Average Collision test took " << (float) (sph.tcollision.count()/sph.n_step) << " ms" << std::endl;
                std::cout << "Average Render spheres took " << (float) (sph.Trender.count()/sph.n_step) << " ms" << std::endl;
              std::cout << "Total  Time: " << task_timer_over_all.time() << " seconds " << std::endl;
              glfwSetWindowShouldClose(window, true);
              exit(0);
            break;
              case GLFW_KEY_SPACE:
            running = !running;
            lastTime = glfwGetTime();
            if(sph.tinit==false)
            {
                sph.tinit=true;
                sph.tsum.zero();
                sph.tforce.zero();
                sph.tcollision.zero();
                sph.trender.zero();
                sph.tpressure.zero();
                task_timer_over_all.start();
            }
            break;
            case GLFW_KEY_R:
                if(imgok) sph.initFromImage();
                else sph.init();
            break;
        case GLFW_KEY_S:
            sph.pforce=!sph.pforce;
            if(sph.pforce)
                std::cout << "pulsatil force = ON" << std::endl;
            else
                std::cout << "pulsatil force = OFF  " << std::endl;
        break;
        case GLFW_KEY_I:
            saveImageFile=!saveImageFile;
        break;
        case GLFW_KEY_L:
            polygonok=!polygonok;
        break;
            case GLFW_KEY_0:
                set =0;
                // set preset parameter values
                std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
                sph.parameters(parameters[set].params);
                if(imgok) sph.initFromImage();
                else sph.init();
                niter=0;
                sph.params.frame_dt=0.01;
            break;
        case GLFW_KEY_1:
            set =1;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            if(imgok) sph.initFromImage();
            else sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_2:
            set =2;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            if(imgok) sph.initFromImage();
            else sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_3:
            set =3;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            if(imgok) sph.initFromImage();
            else sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_4:
            set =4;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            if(imgok) sph.initFromImage();
            else sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_5:
            set =5;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            if(imgok) sph.initFromImage();
            else sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_6:
            set =6;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            if(imgok) sph.initFromImage();
            else sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_7:
            set =7;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            if(imgok) sph.initFromImage();
            else sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_8:
            set =8;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            if(imgok) sph.initFromImage();
            else sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_9:
            set =9;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            if(imgok) sph.initFromImage();
            else sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_G:
                gridok=!gridok;
        break;
        case GLFW_KEY_C:
                curveok=!curveok;
        break;
        case GLFW_KEY_P:
                pok=!pok;
            break;
        case GLFW_KEY_V:
                vcolorok=!vcolorok;
        break;
        case GLFW_KEY_TAB:
            rendermode=(rendermode+1)%4;
        }
    }
}

void renderPolygonObstacle()
{
    //sph.drawSurface();
    if(polygonVBO==0)
    {
        glGenVertexArrays(1, &polygonVAO);
        glGenBuffers(1, &polygonVBO);
        glGenBuffers(1, &polygonEBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).



    }
    glBindVertexArray(polygonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, polygonVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*sph.polygonVertex.size(), &sph.polygonVertex[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, polygonEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*sph.polygonIndex.size(), &sph.polygonIndex[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
    projection = glm::perspective(glm::radians( fov), (float)sph.winWidth / (float)sph.winHeight, 0.1f, 100.0f);
    int projectionLoc = glGetUniformLocation(shaderPolygonProgram, "projection");
    glUseProgram(shaderPolygonProgram);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    int viewLoc = glGetUniformLocation(shaderPolygonProgram, "view");
    glUseProgram(shaderPolygonProgram);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    model = glm::mat4(1.0f);
    int modelLoc = glGetUniformLocation(shaderPolygonProgram, "model");
    glUseProgram(shaderPolygonProgram);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glEnable(GL_LINE_WIDTH);
    glLineWidth(3);
    // draw our first triangle
            glUseProgram(shaderPolygonProgram);
            glBindVertexArray(polygonVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
            //glDrawArrays(GL_TRIANGLES, 0, 6);
            glDrawArrays(GL_LINE_STRIP,0, sph.polygonIndex.size());
            // glBindVertexArray(0); // no need to unbind it every time
    glDisable(GL_LINE_WIDTH);

}


// renders (and builds at first invocation) a sphere
// -------------------------------------------------
void renderSphere(glm::vec3 &pos,glm::vec4 &color)
{
    sphereShader.use();
    //projection = glm::perspective(glm::radians(Interactor->fov), (float)screenW / (float)screenH, 0.1f, 100.0f);
    sphereShader.setMat4("projection", projection);
    sphereShader.setVec4("albedo", glm::vec4(color.x,color.y,color.z,color.w));
    sphereShader.setMat4("view", view);

    model = glm::mat4(1.0f);
    //model = glm::mat4(Interactor->orientation);
    model = glm::translate(model, glm::vec3(pos.x,pos.y,pos.z));
    //model = glm::translate(model, lightPos);
    //model = glm::scale(model, glm::vec3(sph.sphereRadius));
    //model =glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0, 1.0, 0.0));
    sphereShader.setMat4("model",model);
    sphereShader.setVec3("camPos", camera->Position);
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 4;
        const unsigned int Y_SEGMENTS = 4;
        const float PI = 3.14159265359;
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = sph.h*0.2*(std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI));
                float yPos = sph.h*0.2*(std::cos(ySegment * PI));
                float zPos = sph.h*0.2*(std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI));

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = indices.size();

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        float stride = (8) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
     glBindVertexArray(0); // no need to unbind it every time
}


void renderImplicitCurve()
{
    sph.drawSurface();
    if(curveVBO==0)
    {
        glGenVertexArrays(1, &curveVAO);
        glGenBuffers(1, &curveVBO);
        glGenBuffers(1, &curveEBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).



    }
    glBindVertexArray(curveVAO);
    glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*sph.curveVertex.size(), &sph.curveVertex[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, curveEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*sph.curveIndex.size(), &sph.curveIndex[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
    projection = glm::perspective(glm::radians( fov), (float)sph.winWidth / (float)sph.winHeight, 0.1f, 100.0f);
    int projectionLoc = glGetUniformLocation(shaderCurveProgram, "projection");
    glUseProgram(shaderCurveProgram);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    int viewLoc = glGetUniformLocation(shaderCurveProgram, "view");
    glUseProgram(shaderCurveProgram);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    model = glm::mat4(1.0f);
    int modelLoc = glGetUniformLocation(shaderCurveProgram, "model");
    glUseProgram(shaderCurveProgram);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glEnable(GL_LINE_WIDTH);
    glLineWidth(2);
    // draw our first triangle
            glUseProgram(shaderCurveProgram);
            glBindVertexArray(curveVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
            //glDrawArrays(GL_TRIANGLES, 0, 6);
            glDrawArrays(GL_LINES,0, sph.curveIndex.size());
            // glBindVertexArray(0); // no need to unbind it every time
    glDisable(GL_LINE_WIDTH);

}




void renderParticles()
{
    uint np =sph.n_particles();
    auto t1 = std::chrono::high_resolution_clock::now();




    if(firstrender)
    {
        //translations = new glm::vec3[np];
        //instanceColors = new glm::vec4[np];
        //scalarField= new float[np];
        //ParticlesContainer = new ViewParticle[np];
        glGenVertexArrays(1, &particlesVAO);
        glGenBuffers(1, &particlesScalarVBO);
        glGenBuffers(1, &particlesVelocitiesVBO);
        glGenBuffers(1, &particlesVBO);



        //color_map = reinterpret_cast<float const*>(colormap_jet);

        firstrender=false;
    }
    TColorRGBA Aux_color;
    //#pragma omp parallel for
    for(uint i=0; i<np; i++){
        Aux_color.R =sph.particles.C[i][0];
        Aux_color.G =sph.particles.C[i][1];
        Aux_color.B =sph.particles.C[i][2];
        Aux_color.A =sph.particles.C[i][3];
        //sph.NormalizedColor(sph.particles.T[i],&Aux_color);
        sph.particles.C[i][0]=Aux_color.R;
        sph.particles.C[i][1]=Aux_color.G;
        sph.particles.C[i][2]=Aux_color.B;
        sph.particles.C[i][3]=Aux_color.A;
    }
    glBindVertexArray(particlesVAO);
    glm::vec4 c;
    //glm::vec4 ci(0.0f,0.0f,0.0f,0.0f);
    TColorRGBA Dcolor;
    m_shader_scalar.use();
    if(vcolorok)
    {
        m_shader_scalar.setBool("vok",true);
        Dcolor = white;
    }
    else
    {
        Dcolor=lyellow;
        m_shader_scalar.setBool("vok",false);
    }

  /* if(imgok)
       m_shader_scalar.setBool("imageok",true);
   else
       m_shader_scalar.setBool("imageok",false);*/

    if(selected.size()>0)
    {
        instanceColors = new glm::vec4[selected.size()];
        for(uint i=0;i<selected.size();i++)
        {
            instanceColors[i][0] = sph.particles.C[selected[i].idx][0];
            instanceColors[i][1] = sph.particles.C[selected[i].idx][1];
            instanceColors[i][2] = sph.particles.C[selected[i].idx][2];
            instanceColors[i][3] = sph.particles.C[selected[i].idx][3];
            sph.particles.C[selected[i].idx][0]= Dcolor.R;
            sph.particles.C[selected[i].idx][1]= Dcolor.G;
            sph.particles.C[selected[i].idx][2]= Dcolor.B;
            sph.particles.C[selected[i].idx][3]= Dcolor.A;
        }
    }


    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    m_shader_scalar.use();
    m_shader_scalar.setFloat("viewport_width", (float)viewport[2]);

    m_shader_scalar.setFloat("min_scalar", sph.NormMinV);
    m_shader_scalar.setFloat("max_scalar", sph.NormMaxV);
   // m_shader_scalar.setVec4("color",c);

    /*glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, m_textureMap);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256u, 0, GL_RGB, GL_FLOAT, color_map);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_1D);
*/

    glEnable(GL_DEPTH_TEST);
    // Point sprites do not have to be explicitly enabled since OpenGL 3.2 where
    // they are enabled by default. Moreover GL_POINT_SPRITE is deprecate and only
    // supported before OpenGL 3.2 or with compatibility profile enabled.
    glEnable(GL_POINT_SPRITE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointParameterf(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);




    glBindBuffer(GL_ARRAY_BUFFER, particlesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(std::array<float, 3>) * np, &(sph.particles.x)[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, particlesScalarVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(std::array<float, 4>) * np, &(sph.particles.C)[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, particlesVelocitiesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(std::array<float, 3>) * np, &(sph.particles.v)[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);



    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,particlesVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER,particlesScalarVBO);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4* sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER,particlesVelocitiesVBO);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3* sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    //m_shader_scalar.use();
    glBindVertexArray(particlesVAO);
    glDrawArrays(GL_POINTS, 0, np);
    glBindVertexArray(0);

   glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    //glBindTexture(GL_TEXTURE_1D, 0);
    glUseProgram(0);

    if(selected.size()>0)
    {
        //instanceColors = new glm::vec4[selected.size()];
        for(uint i=0;i<selected.size();i++)
        {
            sph.particles.C[selected[i].idx][0] = instanceColors[i][0];
            sph.particles.C[selected[i].idx][1] = instanceColors[i][1];
            sph.particles.C[selected[i].idx][2] = instanceColors[i][2];
            sph.particles.C[selected[i].idx][3] = instanceColors[i][3];
        }
        delete instanceColors;
    }

    std::chrono::duration<double> diff= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-t1);
    sph.Trender +=diff;
}

void renderVectorField()
{
    uint np =sph.n_particles();
    auto t1 = std::chrono::high_resolution_clock::now();




    if(firstVFrender)
    {

        glGenVertexArrays(1, &vectorFieldVAO);
        glGenBuffers(1, &vectorFieldScalarVBO);
        glGenBuffers(1, &vectorFieldVelocitiesVBO);
        glGenBuffers(1, &vectorFieldVBO);
        // framebuffer configuration
        // -------------------------

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        // create a color attachment texture

        glGenTextures(1, &textColorbuffer);
        glBindTexture(GL_TEXTURE_2D, textColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sph.winWidth, sph.winHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textColorbuffer, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glGenVertexArrays(1, &quadVAOScreen);
        glGenBuffers(1, &quadVBOScreen);
        glBindVertexArray(quadVAOScreen);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBOScreen);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        ScreenBuffer.init("../resources/shaders/framebuffers_screen.vs", "../resources/shaders/framebuffers_screen.fs");
        ScreenBuffer.use();
        ScreenBuffer.setInt("screenTexture", 0);
        ScreenBuffer.setVec3("iResolution", glm::vec3(sph.winWidth,sph.winHeight,0.0));

        firstVFrender=false;
    }
    glBindVertexArray(vectorFieldVAO);
    glm::vec4 c;
    //glm::vec4 ci(0.0f,0.0f,0.0f,0.0f);
    TColorRGBA Dcolor;
    vectorFieldShader.use();
    //ScreenBuffer.use();
    glm::vec4 p2;
    if(vcolorok)
    {
        vectorFieldShader.setBool("vok",true);
        Dcolor = white;
        //p2 = projection*view*glm::vec4(sph.particles.x[sph.NormMaxVid][0],sph.particles.x[sph.NormMaxVid][1],sph.particles.x[sph.NormMaxVid][2],1.0);
        //std::cout << "p2.x = " << p2.x << " p2.y = " << p2.y << std::endl;
        //ScreenBuffer.setVec2("MaxvCoords",glm::vec2(p2.x,p2.y));
    }
    else
    {
        Dcolor=lyellow;
        vectorFieldShader.setBool("vok",false);
        ScreenBuffer.setVec2("MaxvCoords",glm::vec2(sph.winWidth*0.5,sph.winHeight*0.5));
    }



    if(selected.size()>0)
    {
        instanceColors = new glm::vec4[selected.size()];
        for(uint i=0;i<selected.size();i++)
        {
            instanceColors[i][0] = sph.particles.C[selected[i].idx][0];
            instanceColors[i][1] = sph.particles.C[selected[i].idx][1];
            instanceColors[i][2] = sph.particles.C[selected[i].idx][2];
            instanceColors[i][3] = sph.particles.C[selected[i].idx][3];
            sph.particles.C[selected[i].idx][0]= Dcolor.R;
            sph.particles.C[selected[i].idx][1]= Dcolor.G;
            sph.particles.C[selected[i].idx][2]= Dcolor.B;
            sph.particles.C[selected[i].idx][3]= Dcolor.A;
        }
    }


    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glad_glClearDepth(0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
    //glClear( GL_DEPTH_BUFFER_BIT);

    vectorFieldShader.use();
    vectorFieldShader.setFloat("viewport_width", (float)viewport[2]);

    vectorFieldShader.setFloat("min_scalar", sph.NormMinV);
    vectorFieldShader.setFloat("max_scalar", sph.NormMaxV);


    //glEnable(GL_DEPTH_TEST);
    // Point sprites do not have to be explicitly enabled since OpenGL 3.2 where
    // they are enabled by default. Moreover GL_POINT_SPRITE is deprecate and only
    // supported before OpenGL 3.2 or with compatibility profile enabled.
    glEnable(GL_POINT_SPRITE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointParameterf(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);




    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(std::array<float, 3>) * np, &(sph.particles.x)[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldScalarVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(std::array<float, 4>) * np, &(sph.particles.C)[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldVelocitiesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(std::array<float, 3>) * np, &(sph.particles.v)[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);



    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,vectorFieldVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER,vectorFieldScalarVBO);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4* sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER,vectorFieldVelocitiesVBO);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3* sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);



    glBindVertexArray(vectorFieldVAO);
    glDrawArrays(GL_POINTS, 0, np);
    glBindVertexArray(0);

   glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);


    glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    // clear all relevant buffers
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);



    ScreenBuffer.use();
    if(vcolorok)
    {
        p2 = projection*view*glm::vec4(sph.particles.x[sph.NormMaxVid][0],sph.particles.x[sph.NormMaxVid][1],sph.particles.x[sph.NormMaxVid][2],1.0);
        //std::cout << "p2.x = " << (p2.x/p2.w+1.0)*0.5 << ", p2.y = " << (p2.y/p2.w+1.0)/1.5 << ", MaxNormVid = " << std::endl;
        ScreenBuffer.setVec2("MaxvCoords",glm::vec2((p2.x/p2.w+1.0)*0.5,(p2.y/p2.w+1.0)/1.5));
    }
    else
        ScreenBuffer.setVec2("MaxvCoords",glm::vec2(0.5,0.5));
    glBindVertexArray(quadVAOScreen);
    glBindTexture(GL_TEXTURE_2D, textColorbuffer);	// use the color attachment texture as the texture of the quad plane
    glDrawArrays(GL_TRIANGLES, 0, 6);

    if(selected.size()>0)
    {
        //instanceColors = new glm::vec4[selected.size()];
        for(uint i=0;i<selected.size();i++)
        {
            sph.particles.C[selected[i].idx][0] = instanceColors[i][0];
            sph.particles.C[selected[i].idx][1] = instanceColors[i][1];
            sph.particles.C[selected[i].idx][2] = instanceColors[i][2];
            sph.particles.C[selected[i].idx][3] = instanceColors[i][3];
        }
        delete instanceColors;
    }

    std::chrono::duration<double> diff= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-t1);
    sph.Trender +=diff;
}

void renderVectorFieldColor()
{
    uint np =sph.n_particles();
    auto t1 = std::chrono::high_resolution_clock::now();




    if(firstVFrenderColor)
    {
        //translations = new glm::vec3[np];
        //instanceColors = new glm::vec4[np];
        //scalarField= new float[np];
        //ParticlesContainer = new ViewParticle[np];
        glGenVertexArrays(1, &vectorFieldColorVAO);
        glGenBuffers(1, &vectorFieldColorScalarVBO);
        glGenBuffers(1, &vectorFieldColorVelocitiesVBO);
        glGenBuffers(1, &vectorFieldColorVBO);



        //color_map = reinterpret_cast<float const*>(colormap_jet);

        firstVFrenderColor=false;
    }
    glBindVertexArray(vectorFieldColorVAO);
    glm::vec4 c;
    //glm::vec4 ci(0.0f,0.0f,0.0f,0.0f);
    TColorRGBA Dcolor;
    vectorFieldShaderColor.use();
    if(vcolorok)
    {
        vectorFieldShaderColor.setBool("vok",true);
        Dcolor = white;
    }
    else
    {
        Dcolor=lyellow;
        vectorFieldShaderColor.setBool("vok",false);
    }



    if(selected.size()>0)
    {
        instanceColors = new glm::vec4[selected.size()];
        for(uint i=0;i<selected.size();i++)
        {
            instanceColors[i][0] = sph.particles.C[selected[i].idx][0];
            instanceColors[i][1] = sph.particles.C[selected[i].idx][1];
            instanceColors[i][2] = sph.particles.C[selected[i].idx][2];
            instanceColors[i][3] = sph.particles.C[selected[i].idx][3];
            sph.particles.C[selected[i].idx][0]= Dcolor.R;
            sph.particles.C[selected[i].idx][1]= Dcolor.G;
            sph.particles.C[selected[i].idx][2]= Dcolor.B;
            sph.particles.C[selected[i].idx][3]= Dcolor.A;
        }
    }


    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    vectorFieldShaderColor.use();
    vectorFieldShaderColor.setFloat("viewport_width", (float)viewport[2]);

    vectorFieldShaderColor.setFloat("min_scalar", sph.NormMinV);
    vectorFieldShaderColor.setFloat("max_scalar", sph.NormMaxV);
   // m_shader_scalar.setVec4("color",c);

    /*glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, m_textureMap);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256u, 0, GL_RGB, GL_FLOAT, color_map);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_1D);
*/

    glEnable(GL_DEPTH_TEST);
    // Point sprites do not have to be explicitly enabled since OpenGL 3.2 where
    // they are enabled by default. Moreover GL_POINT_SPRITE is deprecate and only
    // supported before OpenGL 3.2 or with compatibility profile enabled.
    glEnable(GL_POINT_SPRITE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointParameterf(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);




    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldColorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(std::array<float, 3>) * np, &(sph.particles.x)[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldColorScalarVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(std::array<float, 4>) * np, &(sph.particles.C)[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldColorVelocitiesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(std::array<float, 3>) * np, &(sph.particles.v)[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);



    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,vectorFieldColorVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER,vectorFieldColorScalarVBO);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4* sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER,vectorFieldColorVelocitiesVBO);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3* sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    //m_shader_scalar.use();
    glBindVertexArray(vectorFieldColorVAO);
    glDrawArrays(GL_POINTS, 0, np);
    glBindVertexArray(0);

   glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    //glBindTexture(GL_TEXTURE_1D, 0);
    glUseProgram(0);

    if(selected.size()>0)
    {
        //instanceColors = new glm::vec4[selected.size()];
        for(uint i=0;i<selected.size();i++)
        {
            sph.particles.C[selected[i].idx][0] = instanceColors[i][0];
            sph.particles.C[selected[i].idx][1] = instanceColors[i][1];
            sph.particles.C[selected[i].idx][2] = instanceColors[i][2];
            sph.particles.C[selected[i].idx][3] = instanceColors[i][3];
        }
        delete instanceColors;
    }

    std::chrono::duration<double> diff= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-t1);
    sph.Trender +=diff;
}


void renderParticlesInstance()
{


    uint np =sph.n_particles();
    auto t1 = std::chrono::high_resolution_clock::now();

    instanceShader.use();

    // instanceShader.setVec4("Color", glm::vec4(c[0],c[1],c[2],c[3]));
    if(instanceVBO==0)
    {
        particleTexture.loadTextureFromFile("../resources/textures/smoke_particle.png",true);
        //particleTexture.load_texture("../resources/textures/smoke_particle.png");
        translations = new glm::vec3[np];
        instanceColors = new glm::vec4[np];
        ParticlesContainer = new ViewParticle[np];
        glGenBuffers(1, &instanceVBO);
        glGenBuffers(1, &instancecolorVBO);

        static const GLfloat g_vertex_buffer_data[] = {
            -Sc, -Sc, 0.0f, 0.0f, 0.0f,
            Sc, -Sc, 0.0f, 1.0f, 0.0f,
            -Sc, Sc, 0.0f, 0.0f, 1.0f,
            Sc, Sc, 0.0f, 1.0f, 1.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));



    }
    TColorRGBA Dcolor;
    if(vcolorok)
    {
        Dcolor = white;
    }
    else
        Dcolor=lyellow;


    //#pragma omp parallel for
    for(uint i=0; i<np; i++){
        uint idx=i;
        glm::vec4 c;
        glm::vec4 ci(0.0f,0.0f,0.0f,0.0f);
        c[0]=sph.particles.C[idx][0];
        c[1]=sph.particles.C[idx][1];
        c[2]=sph.particles.C[idx][2];
        c[3]=sph.particles.C[idx][3];
        float vx,vy,vz;
        vx = sph.particles.v[idx][0];
        vy = sph.particles.v[idx][1];
        vz = sph.particles.v[idx][2];
        //std::cout << "v["<<idx<<"] = ("<<vx <<", "<<vy<<", "<<vz<<") " << std::endl;
        Real v = glm::l2Norm(glm::vec3(vx,vy,vz));
        if(!imgok)
        {
            v = static_cast<Real>(1.0)*((v - sph.NormMinV) / (sph.NormMaxV - sph.NormMinV));
            c[3]= max(v,static_cast<Real>(0.2));
        }
        if(vcolorok)
        {

        v = min(static_cast<Real>(0.75)*v, static_cast<Real>(0.75));
         hsvToRgb(0.75- (float)v, 1.0f, 1.0f, c);
        }
        else
        {
            if(!imgok)
            {
                v = min(static_cast<Real>(0.9)*v, static_cast<Real>(0.9));
                c[1] = min(static_cast<Real>(0.8),static_cast<Real>(1-v));
            }

        }
        ParticlesContainer[idx].pos = glm::vec3(sph.particles.x[idx][0],sph.particles.x[idx][1],sph.particles.x[idx][2]);
        ParticlesContainer[idx].Color= c;
        ViewParticle& p = ParticlesContainer[idx];

            instanceColors[idx] = p.Color;

        translations[idx] = p.pos;
       // p.cameradistance = glm::length2( p.pos -  camera->Position);

    }

    if(selected.size()>0)
        for(uint i=0;i<selected.size();i++)
        {
            instanceColors[selected[i].idx][0]= Dcolor.R;
            instanceColors[selected[i].idx][1]= Dcolor.G;
            instanceColors[selected[i].idx][2]= Dcolor.B;
            instanceColors[selected[i].idx][3]= Dcolor.A;
        }


    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * np, &translations[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, instancecolorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * np, &instanceColors[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    particleTexture.Bind();
    // also set instance data
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, instancecolorVBO); // this attribute comes from a different vertex buffer
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(3, 1); // tell OpenGL this is an instanced vertex attribute.


    glBindVertexArray(quadVAO);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, np);
    glBindVertexArray(0);






    std::chrono::duration<double> diff= std::chrono::high_resolution_clock::now()-t1;
    sph.Trender +=diff;
}


static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

     //   std::cout<< "xpos =" << xpos << " ypos =" << ypos <<std::endl;


    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;
    glm::vec2 screen_pos,pixel_pos;
    glm::vec3 win,world1,world0,world;
    int screen_w,screen_h,pixel_w,pixel_h;
   if((mouse_left)&& (!polygonok))
    {
        //float x = (2.0f * xpos) / sph.winWidth - 1.0f;
        //float y = 1.0f - (2.0f * ypos) / sph.winHeight;

       glfwGetWindowSize(window, &screen_w, &screen_h); // better use the callback and cache the values
       glfwGetFramebufferSize(window, &pixel_w, &pixel_h); // better use the callback and cache the values

        screen_pos=glm::vec2(xpos, ypos);
        pixel_pos=screen_pos * glm::vec2(pixel_w, pixel_h) / glm::vec2(screen_w, screen_h); // note: not necessarily integer
       pixel_pos = pixel_pos + glm::vec2(0.5f, 0.5f); // shift to GL's center convention
        win=glm::vec3(pixel_pos.x, pixel_h-pixel_pos.y, 0.0f);
      glReadPixels((GLint)win.x, (GLint)win.y,1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win.z);

        glm::vec4 viewport(0.0f,0.0f,(float)screen_w, (float)screen_h);

         projection = glm::perspective(glm::radians( fov), (float)sph.winWidth / (float)sph.winHeight, 0.1f, 100.0f);

         world1 = glm::unProject(win, view, projection,viewport);
        win.z=0.0f;
         world0 = glm::unProject(win, view, projection,viewport);
        float t=-world0.z/(world1.z-world0.z);
         world = world0+t*(world1-world0);
        glm::vec3 point(world.x,world.y,0.0f);
        sph.selectPoints(point, selected);
        for (int i = 0; i < selected.size(); ++i) {
          glm::vec4 &pa = sph.particles.a[selected[i].idx];
          //p.T+=0.1;
          sph.particles.T[selected[i].idx]+=0.2;
          float k = sph.vkernel.value(selected[i].d, selected[i].d_squared);
          glm::vec4 disp(xoffset,yoffset,0.0f,0.0f);
          pa += mouse_force * k * disp;
        }

    }

   if((mouse_left)&& (mouse_right))
    {
       glfwGetWindowSize(window, &screen_w, &screen_h); // better use the callback and cache the values
       glfwGetFramebufferSize(window, &pixel_w, &pixel_h); // better use the callback and cache the values

        screen_pos=glm::vec2(xpos, ypos);
        pixel_pos=screen_pos * glm::vec2(pixel_w, pixel_h) / glm::vec2(screen_w, screen_h); // note: not necessarily integer
       pixel_pos = pixel_pos + glm::vec2(0.5f, 0.5f); // shift to GL's center convention
        win=glm::vec3(pixel_pos.x, pixel_h-pixel_pos.y, 0.0f);
      glReadPixels((GLint)win.x, (GLint)win.y,1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win.z);

        glm::vec4 viewport(0.0f,0.0f,(float)screen_w, (float)screen_h);

         projection = glm::perspective(glm::radians( fov), (float)sph.winWidth / (float)sph.winHeight, 0.1f, 100.0f);

         world1 = glm::unProject(win, view, projection,viewport);
        win.z=0.0f;
         world0 = glm::unProject(win, view, projection,viewport);
        float t=-world0.z/(world1.z-world0.z);
         world = world0+t*(world1-world0);
        glm::vec3 point(world.x,world.y,0.0f);
        sph.polygonVertex.push_back(point);
        //if(sph.polygonVertex.size()>2)
            //sph.polygonIndex.push_back(polygonIndex-1);
        sph.polygonIndex.push_back(polygonIndex);
        std::cout << "polygonIndex = " << polygonIndex <<std::endl;
        polygonIndex++;
         mouse_left = false;
    }
 }

void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action==GLFW_PRESS) {


        if (button == GLFW_MOUSE_BUTTON_RIGHT)
            mouse_right = true;

        if (button == GLFW_MOUSE_BUTTON_LEFT)
            mouse_left = true;



    }
    else if (action==GLFW_RELEASE)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            mouse_left = false;
            selected.clear();

        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT)
            mouse_right = false;
    }
}

void printMatrix(glm::mat4x4 M)
{
    std::cout << "[" << M[0][0] <<" "  << M[1][0] <<" " << M[2][0] <<" " << M[3][0] <<" ]" <<std::endl;
    std::cout << "[" << M[0][1] <<" "  << M[1][1] <<" " << M[2][1] <<" " << M[3][1] <<" ]" <<std::endl;
    std::cout << "[" << M[0][2] <<" "  << M[1][2] <<" " << M[2][2] <<" " << M[3][2] <<" ]" <<std::endl;
    std::cout << "[" << M[0][3] <<" "  << M[1][3] <<" " << M[2][3] <<" " << M[3][3] <<" ]" <<std::endl;
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
};





void initImGUI(GLFWwindow* window)
{
    my_tex_id = (void*) loadTexture(FileSystem::getPath("resources/textures/texture.jpg").c_str());
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui::GetStyle().FrameBorderSize=2;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}


void renderGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //ImGui::ShowDemoWindow();

#define IMGUI_WINDOW_WIDTH 265
#define IMGUI_WINDOW_HEIGHT 150
#define IMGUI_WINDOW_MARGIN 265
    int tmpHeight,yShift;
   // if(!vcolorok)
    //{
      //  tmpHeight=105;
        //yShift=10;
    //}
     //else
    //{
        tmpHeight=130;
        yShift=30;
    //}
    ImGui::SetNextWindowSize(ImVec2(sph.winWidth-275, tmpHeight));
    ImGui::SetNextWindowSizeConstraints(ImVec2(sph.winWidth-277, tmpHeight), ImVec2(sph.winWidth-277,tmpHeight));
    ImGui::SetNextWindowPos(ImVec2(IMGUI_WINDOW_MARGIN, 0));
    ImGui::Begin("Control Panel");
        //static bool mm = true;
        //ImGui::ShowDemoWindow(&mm);
    //ImGuiIO& io = ImGui::GetIO();
          ImGui::Separator();



        float wrapping=50;
        ImVec2 pos = ImGui::GetCursorScreenPos();
        //ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + wrapping, pos.y), ImVec2(pos.x + wrapping + 10, pos.y + ImGui::GetTextLineHeight()), IM_COL32(255,0,255,255));
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrapping);
        // ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Plane Rot (in degrees)");
        ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,255,0,55));
        ImGui::PopTextWrapPos();


         wrapping=160;
         ImGui::SameLine(100);
         pos = ImGui::GetCursorScreenPos();
         //ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + wrapping, pos.y), ImVec2(pos.x + wrapping + 10, pos.y + ImGui::GetTextLineHeight()), IM_COL32(255,0,255,255));
         ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrapping);
          ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Camera Position");
         ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,255,255,55));
         ImGui::PopTextWrapPos();


         ImGui::Columns(3, "mixed");
         ImGui::Separator();
         ImGui::SetColumnWidth(0,235);
         ImGui::SetColumnWidth(1,210);
         //ImGui::SetNextItemWidth(60);
         /*if (ImGui::DragScalar("Z", ImGuiDataType_Float, &Interactor->planeRotXYZ.z, 1.0f, &f32_zero, &f32_360, "%.4f", 1.0f))
         {
             // bound value entered from user
             if (Interactor->planeRotXYZ.z < f32_zero)
                 Interactor->planeRotXYZ.z = f32_zero;
             else if (Interactor->planeRotXYZ.z > f32_360)
                 Interactor->planeRotXYZ.z = f32_360;
         }
         ImGui::SetNextItemWidth(60);
         ImGui::SameLine(95);
         if (ImGui::DragScalar("Y", ImGuiDataType_Float, &Interactor->planeRotXYZ.y, 1.0f, &f32_neg_180, &f32_pos_180, "%.4f", 1.0f))
         {
             if (Interactor->planeRotXYZ.y < f32_neg_180)
                 Interactor->planeRotXYZ.y = f32_neg_180;
             else if (Interactor->planeRotXYZ.y > f32_pos_180)
                 Interactor->planeRotXYZ.y = f32_pos_180;
         }

         ImGui::SetNextItemWidth(60);
         ImGui::SameLine(180);
         if (ImGui::DragScalar("X", ImGuiDataType_Float, &Interactor->planeRotXYZ.x, 1.0f, &f32_neg_90, &f32_pos_90, "%.4f", 1.0f))
         {
             if (Interactor->planeRotXYZ.z < f32_neg_90)
                 Interactor->planeRotXYZ.z = f32_neg_90;
             else if (Interactor->planeRotXYZ.z > f32_pos_90)
                 Interactor->planeRotXYZ.z = f32_pos_90;
         }*/

        // ImGui::NextColumn();

         //ImGui::SetNextItemWidth(20);
         //ImGui::SameLine(100);

         ImGui::SetNextItemWidth(10);
         //ImGui::SameLine(100);
         ImGui::Text("Pos = (%.01f, %.01f, %.01f)", camera->Position.x, camera->Position.y, camera->Position.z);
         ImGui::NextColumn();
         ImGui::Text("Front = (%.01f, %.01f, %.01f)", camera->Front.x, camera->Front.y, camera->Front.z);
         ImGui::NextColumn();
         ImGui::Text("Up = (%.01f, %.01f, %.01f)", camera->Up.x, camera->Up.y, camera->Up.z);
         //ImGui::Separator()


        ImGui::Columns(3, "mixed");
        ImGui::Separator();
        ImGui::SetColumnWidth(0,235);
        ImGui::SetColumnWidth(1,210);
        ImGui::SetNextItemWidth(60);

        ImGui::NextColumn();


        ImGui::Columns(1);
        ImGui::Separator();


        //ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Misc");

        // wireframe mode
        // TODO: migrate to use shader to draw wireframe instead of using fixed-function ...
        //ImGui::Checkbox("Wireframe mode", &Interactor->wireframeMode);




        //ImGui::Separator();
        //ImGui::SetColumnWidth(0,250);
        wrapping=120;
        pos = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRect(ImVec2(pos.x, pos.y-3), ImVec2(pos.x + 258 , pos.y + ImGui::GetTextLineHeight()+yShift), IM_COL32(220,220,220,255));
        ImGui::GetWindowDrawList()->AddRect(ImVec2(pos.x+258, pos.y-3), ImVec2(sph.winWidth-27 , pos.y + ImGui::GetTextLineHeight()+yShift), IM_COL32(220,220,220,255));
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrapping);
         ImGui::Text("Yaw: (%.02f)", camera->Yaw);
        ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,255,255,55));
        ImGui::PopTextWrapPos();

        ImGui::SameLine(120);

        pos = ImGui::GetCursorScreenPos();
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrapping);
        ImGui::Text("Pitch: (%.02f)", camera->Pitch);
        ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,255,255,55));
        ImGui::PopTextWrapPos();


        if(vcolorok)
        {


            float my_tex_w = 250;
            float my_tex_h = 20;

            ImGui::SameLine(350);
            ImGui::Text("Min = %.2f", sph.NormMinV); ImGui::SameLine(600); ImGui::Text("Max = %.2f", sph.NormMaxV);
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::Text("");
            ImGui::SameLine(280);
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Vel (Norm)");
            ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,255,255,55));
            ImGui::SameLine(400);
            if(rendermode==0)
            {
                ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetItemRectMax().x+50,ImGui::GetItemRectMin().y), ImVec2(ImGui::GetItemRectMax().x+302,ImGui::GetItemRectMax().y+9),IM_COL32(50,125,255,255),IM_COL32(255,25,25,245),IM_COL32(255,25,25,25),IM_COL32(50,125,255,255));
                ImGui::GetWindowDrawList()->AddRect(ImVec2(ImGui::GetItemRectMax().x+50,ImGui::GetItemRectMin().y), ImVec2(ImGui::GetItemRectMax().x+302,ImGui::GetItemRectMax().y+9),IM_COL32(155,155,155,255));
            }
            else
                ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), ImVec2(0,0), ImVec2(1,1), ImVec4(1.0f,1.0f,1.0f,1.0f), ImVec4(1.0f,1.0f,1.0f,0.5f));
        }
        else
        {
            ImGui::SameLine(350);
            ImGui::Text("Min = %.2f", sph.Tmin); ImGui::SameLine(600); ImGui::Text("Max = %.2f", sph.Tmax);
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::Text("");
            ImGui::SameLine(280);
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Vel (Norm)");
            ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,255,255,55));
            ImGui::SameLine(400);
            ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetItemRectMax().x+50,ImGui::GetItemRectMin().y), ImVec2(ImGui::GetItemRectMax().x+302,ImGui::GetItemRectMax().y+9),IM_COL32(55,255,255,255),IM_COL32(0,10,255,255),IM_COL32(0,10,255,255),IM_COL32(55,(int)(0.8*255),255,255));
            ImGui::GetWindowDrawList()->AddRect(ImVec2(ImGui::GetItemRectMax().x+50,ImGui::GetItemRectMin().y), ImVec2(ImGui::GetItemRectMax().x+302,ImGui::GetItemRectMax().y+9),IM_COL32(155,155,155,255));
        }

        //ImGui::Columns(1);
        //ImGui::Separator();


    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
