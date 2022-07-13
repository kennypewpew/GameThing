#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <thread>
#include <iostream>
#include <vector>

const char* vertexSource = R"glsl(
    #version 460 core

    in vec3 position;
    uniform mat4 view;

    void main()
    {
        gl_Position = view * vec4(position, 1.0);
    }
)glsl";

const char* fragmentSource = R"glsl(
    #version 460 core
    
    out vec4 outColor;
    
    void main()
    {
        outColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
)glsl";

void CheckUniforms( GLuint shaderProgram ) {
  GLint count;
  GLint size; // size of the variable
  GLenum type; // type of the variable (float, vec3 or mat4, etc)
  
  const GLsizei bufSize = 16; // maximum name length
  GLchar name[bufSize]; // variable name in GLSL
  GLsizei length; // name length
  glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &count);
  std::cout << count << "\n";
  for ( int i = 0 ; i < count ; ++i ) {
    glGetActiveUniform(shaderProgram, (GLuint)i, bufSize, &length, &size, &type, name);
    printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
  }
  if ( count == 0 ) printf("No uniforms detected\n");
}

GLFWwindow* InitializeLotsOfThings() {
  glfwInit();
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );

  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow* window = glfwCreateWindow(800, 600, "testing", nullptr, nullptr); // Windowed
  //GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", glfwGetPrimaryMonitor(), nullptr); // Fullscreen

  // Grab active attention
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Have to init glew AFTER grabbing attention
  glewExperimental = GL_TRUE;
  if ( ! GLEW_OK == glewInit() ) {
    std::cout << "Problem initializing GLEW\n";
    exit(1);
  }

  return window;
}

GLuint CompileShaderProgram() {
  // Compile shaders
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader,1,&vertexSource,NULL);
  glCompileShader(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader,1,&fragmentSource,NULL);
  glCompileShader(fragmentShader);

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);

  glBindFragDataLocation(shaderProgram,0,"outColor");
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  GLint posAttrib = glGetAttribLocation(shaderProgram,"position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

  return shaderProgram;
}


void FillVertsAndInds( std::vector<glm::vec3> &vertices
                     , std::vector<GLuint> &indices
                     , const std::string &mapFile
                     ) {
  int xdim = 7;
  int ydim = 5;
  for ( int x = 0 ; x <= xdim ; ++x ) {
    for ( int y = 0 ; y <= ydim ; ++y ) {
      float xx = 1.5*float(x)/float(xdim)-.75;
      float yy = 1.5*float(y)/float(ydim)-.75;
      float zz = int(float(x)/5.);
      vertices.push_back(glm::vec3(xx,yy,zz));
    }
  }
  for ( int x = 0 ; x < xdim ; ++x ) {
    for ( int y = 0 ; y < ydim ; ++y ) {
      indices.push_back((x+0)*(ydim+1) + y+0);
      indices.push_back((x+0)*(ydim+1) + y+1);
      indices.push_back((x+0)*(ydim+1) + y+0);
      indices.push_back((x+1)*(ydim+1) + y+0);
    }
    indices.push_back((x+0)*(ydim+1) + ydim);
    indices.push_back((x+1)*(ydim+1) + ydim);
  }
  for ( int y = vertices.size()-(ydim+1) ; y < vertices.size()-1 ; ++y ) {
    indices.push_back(y);
    indices.push_back(y+1);
  }
}

void RotateView( const std::string &uniformName
               , const float &degrees
               , const glm::vec3 &rotationVector
               , const GLuint &program
               ) {
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::rotate(view,glm::radians(degrees), rotationVector);
    GLint uniTrans = glGetUniformLocation(program, uniformName.c_str());
    glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(view));
}


int main(void) {
  GLFWwindow *window = InitializeLotsOfThings();

  std::vector<glm::vec3> vertices;
  std::vector<GLuint> indices;
  FillVertsAndInds( vertices, indices , "session/default/maps/example.map" );

  // Vertex array needs to exist for things to appear
  GLuint vao;
  glGenVertexArrays(1,&vao);
  glBindVertexArray(vao);

  // Vertices
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), glm::value_ptr(vertices[0]), GL_STATIC_DRAW);

  // Indices
  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*indices.size(), indices.data(),GL_STATIC_DRAW);

  GLuint shaderProgram = CompileShaderProgram();

  while( !glfwWindowShouldClose(window) ) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    RotateView( "view" , 25.f , glm::vec3(0.0,0.2,0.8) , shaderProgram);
    glDrawElements(GL_LINES,indices.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
