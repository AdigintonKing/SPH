#ifdef __APPLE__
#include <GLUT/glut.h>
#else
//#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <iostream>
#include "sph.hh"

SPH sph;
int niter=0;
CGAL::Timer task_timer_over_all;

bool running = false, drawgrid = false, drawsurface = false, drawhash = false, drawtree = false, saveImageFile=false,spheresok=true, polygonok=false;
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
    "   FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
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


unsigned int VBO, VAO, EBO,gridVBO, gridVAO, gridEBO,curveVBO=0, curveVAO, curveEBO,polygonVBO=0, polygonVAO, polygonEBO;
int vertexShader,fragmentShader,shaderProgram,vertexGridShader,fragmentGridShader,shaderGridProgram,vertexCurveShader,fragmentCurveShader,shaderCurveProgram,vertexPolygonShader,fragmentPolygonShader,shaderPolygonProgram;
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
            -0.975f,  1.0f, 0.0f,  // top right
            -0.975f, -0.82f, 0.0f,  // bottom right
            0.975f, -0.82f, 0.0f,  // bottom left
           0.975f,  1.0f, 0.0f   // top left
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
    unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);
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

const int MaxParticles = 20000;
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
void renderImplicitCurve();
void renderPolygonObstacle();
void initImGUI(GLFWwindow* window);
void renderGUI();
glm::mat4 projection;
glm::vec3 *translations;
glm::vec4 *instanceColors;
/*************/

//glm::mat4 model;
unsigned int instanceVBO=0;
uint instancecolorVBO=0;
unsigned int  quadVAO,quadVBO;
Texture2D  particleTexture;
static int ox, oy;
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
{"incompressible", {0.001, 2.5, 7, 1000, 998.6, 0.8, 0.002, 0.04, false, Vector(0, -9.81, 0), 0.7}},
{"compressible", {0.001, 2.5, 1, 500, 200.6, 0.8, 0.02, 0.4, true, Vector(0, -9.81, 0), 0.7}},
{"real XSPH", {0.001, 2.5, 7, 1000, 998.6, 0.8, 0.002, 0.04, true, Vector(0, -9.81, 0), 0.7}},
{"fake XSPH high", {0.001, 2.5, 7, 1000, 998.6, 0.8, 0.002, 0.8, false, Vector(0, -9.81, 0), 0.7}},
{"viscosity too low", {0.001, 2.5, 7, 1000, 998.6, 0.8, 0.001, 0.04, false, Vector(0, -9.81, 0), 0.7}},
{"viscosity ok (low)", {0.001, 2.5, 7, 1000, 998.6, 0.8, 0.003, 0.04, false, Vector(0, -9.81, 0), 0.7}},
{"viscosity highest", {0.001, 2.5, 7, 1000, 998.6, 0.8, 0.05, 0.04, false, Vector(0, -9.81, 0), 0.7}},
{"viscosity too high", {0.001, 2.5, 7, 1000, 998.6, 0.8, 0.1, 0.04, false, Vector(0, -9.81, 0), 0.7}},
{"low smoothing radius", {0.001, 1.8, 7, 1000, 998.6, 0.8, 0.001, 0.04, false, Vector(0, -9.81, 0), 0.7}},
{"high smoothing radius", {0.001, 5, 7, 1000, 998.6, 0.8, 0.001, 0.04, false, Vector(0, -9.81, 0), 0.7}}
};

void idle()
{

    // render OpenGL here


    if (running) {
    sph.step();
    niter++;
    if(saveImageFile)
    {
      sph.surfaceCells(50);
      sph.WriteScreenImage();
    }


    if (niter % test == 0)
    {

        if(pok)
        renderParticlesInstance();
        glfwSwapBuffers(window);


   // glutPostRedisplay();
    }
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

glm::vec3 lightPos(-0.8f, 0.75f, 2.0f);
glm::vec3 lightPos1(0.8f, 0.75f, 2.0f);

int main(int argc, char ** argv) 
{
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
    sph.init();
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

    camera = new Camera(cameraPos,glm::vec3(0.0f, 1.0f,  0.0f));
    /*camera->Front=glm::normalize(cameraFront);
    camera->Right = glm::normalize(glm::cross(camera->Front, camera->WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    camera->Up    = glm::normalize(glm::cross(camera->Right, camera->Front));*/
    //camera->updateCameraVectors();
    view= camera->calculate_lookAt_matrix(cameraPos,cameraFront,cameraUp);
    instanceShader.init("../resources/properties/particleInstancing.vs", "../resources/properties/particleInstancing.fs");
    instanceShader.use();
    instanceShader.setInt("sprite",255);
    instanceShader.setFloat("radius",sph.h*0.17);
    instanceShader.setVec3("lightPosition",lightPos);
    instanceShader.setVec3("lightPosition1",lightPos1);
    Sc=0.17*sph.h;
    sph.generateGrid();
    compileVertexShaders();
    compileGridShaders();
    compileCurveShaders();
    compilePolygonShaders();
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
     glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ); // polygon drawing mode (GL_POINT, GL_LINE, GL_FILL)
    initImGUI(window);
     double fps;
     char title_string[50];
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.4f, 0.4f, 1.0f);
        //glClearColor(0.17f, 0.152f, 0.129f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
        // input
        // -----
        processInput(window);

        renderGUI();

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
            renderParticlesInstance();
            glfwSwapBuffers(window);
        }
        else
        {
            if(frames > 10) {
            timer.stop();
            fps = (float)frames/timer.time();
            sprintf(title_string, "Visualizador - FPS = %.2f", fps);
            glfwSetWindowTitle(window, title_string);
            timer.restart();
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
                sph.init();
            break;
        case GLFW_KEY_L:
            polygonok=!polygonok;
        break;
            case GLFW_KEY_0:
                set =0;
                // set preset parameter values
                std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
                sph.parameters(parameters[set].params);
                sph.init();
                niter=0;
                sph.params.frame_dt=0.01;
            break;
        case GLFW_KEY_1:
            set =1;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_2:
            set =2;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_3:
            set =3;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_4:
            set =4;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_5:
            set =5;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_6:
            set =6;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_7:
            set =7;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_8:
            set =8;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            sph.init();
            niter=0;
            sph.params.frame_dt=0.01;
        break;
        case GLFW_KEY_9:
            set =9;
            // set preset parameter values
            std::cout << "setting parameter set " << set << ": " << parameters[set].name << std::endl;
            sph.parameters(parameters[set].params);
            sph.init();
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
        case GLFW_KEY_V:
                vcolorok=!vcolorok;
        break;
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
    glLineWidth(3);
    // draw our first triangle
            glUseProgram(shaderCurveProgram);
            glBindVertexArray(curveVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
            //glDrawArrays(GL_TRIANGLES, 0, 6);
            glDrawArrays(GL_LINES,0, sph.curveIndex.size());
            // glBindVertexArray(0); // no need to unbind it every time
    glDisable(GL_LINE_WIDTH);

}

void renderParticlesInstance()
{
    uint np =sph.particles.x.size();
    auto t1 = std::chrono::high_resolution_clock::now();


    instanceShader.use();
    projection = glm::perspective(glm::radians( fov), (float)sph.winWidth / (float)sph.winHeight, 0.1f, 100.0f);
    //projection = glm::ortho(0.0f, (float)sph.winWidth, 0.0f, (float)sph.winHeight);
     instanceShader.setMat4("projection", projection);

     instanceShader.setMat4("view",  view);
     model = glm::mat4(1.0f);
     instanceShader.setMat4("model",model);
     instanceShader.setVec3("camPos",  camera->Position);
    // instanceShader.setVec4("Color", glm::vec4(c[0],c[1],c[2],c[3]));
    if(instanceVBO==0)
    {
        particleTexture.loadTextureFromFile("../resources/textures/Droplet.png",true);
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));



    }
    TColorRGBA Dcolor;
    if(vcolorok)
    {
        Dcolor = white;
    }
    else
        Dcolor=blue;


    #pragma omp parallel for
    for(uint i=0; i<np; i++){
        uint idx=i;
        glm::vec4 c;
        glm::vec4 ci(0.0f,0.0f,0.0f,0.0f);
        c[0]=sph.particles.C[idx].R;
        c[1]=sph.particles.C[idx].G;
        c[2]=sph.particles.C[idx].B;
        c[3]=sph.particles.C[idx].A;
        Real v = sph.particles.v[idx].norm();
        v = static_cast<Real>(1.0)*((v - sph.NormMinV) / (sph.NormMaxV - sph.NormMinV));

        if(vcolorok)
        {

        v = min(static_cast<Real>(0.75)*v, static_cast<Real>(0.74));
         hsvToRgb(0.74- (float)v, 1.0f, 1.0f, c);
        }
        else
        {

            v = min(static_cast<Real>(0.9)*v, static_cast<Real>(0.9));
            c[1] = min(static_cast<Real>(0.8),static_cast<Real>(1-v));
        }
        ParticlesContainer[idx].pos = glm::vec3(sph.particles.x[idx].x,sph.particles.x[idx].y,sph.particles.x[idx].z);
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
        Vector point(world.x,world.y,0.0f);
        sph.selectPoints(point, selected);
        for (int i = 0; i < selected.size(); ++i) {
          Vector &pa = sph.particles.a[selected[i].idx];
          //p.T+=0.1;
          float k = sph.vkernel.value(selected[i].d, selected[i].d_squared);
          Vector disp(xoffset,yoffset,0.0f);
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


#define IMGUI_WINDOW_WIDTH 265
#define IMGUI_WINDOW_HEIGHT 150
#define IMGUI_WINDOW_MARGIN 265
    int tmpHeight,yShift;
    if(!vcolorok)
    {
        tmpHeight=105;
        yShift=10;
    }
     else
    {
        tmpHeight=130;
        yShift=30;
    }
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
         //ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,255,0,55));
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
            ImGui::SameLine(400);
            ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), ImVec2(0,0), ImVec2(1,1), ImVec4(1.0f,1.0f,1.0f,1.0f), ImVec4(1.0f,1.0f,1.0f,0.5f));
        }

        //ImGui::Columns(1);
        //ImGui::Separator();


    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
