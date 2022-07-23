// Basic OpenGL ES 3 + SDL2 template code
#include <SDL.h>
#include <SDL_opengles2.h>
#include <GLES3/gl3.h>
#include <cstdio>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Maps.h"
#include "MapGen.h"
#include "Window.h"

#include <EGL/egl.h>

PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;
PFNGLISVERTEXARRAYOESPROC glIsVertexArrayOES;

const unsigned int DISP_WIDTH = 480;
const unsigned int DISP_HEIGHT = 960;

const unsigned int TARGET_FPS = 30;
const unsigned int TARGET_FRAME_TIME = 1000 / TARGET_FPS;

const char* vertexSource = R"glsl(
    #version 300 es

    precision mediump float;
    in mediump vec3 position;
    uniform mat4 view;

    void main()
    {
        gl_Position = view * vec4(position, 1.0);
    }
)glsl";

const char* fragmentSource = R"glsl(
    #version 300 es

    precision mediump float;
    out mediump vec4 fragColor;
    
    void main()
    {
        fragColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
)glsl";

const char* fragmentSourceWhite = R"glsl(
    #version 300 es
    
    precision mediump float;
    out mediump vec4 fragColor;
    
    void main()
    {
        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
)glsl";

void shaderCompileCheck(GLint s) {
  GLint status;
  glGetShaderiv(s, GL_COMPILE_STATUS, &status);
  if ( status != GL_TRUE ) {
    char buffer[512];
    glGetShaderInfoLog(s, 512, NULL, buffer);
    printf("%s\n",buffer);
  }
}

class Rotation {
 public:
  float degrees;
  glm::vec3 vector;
  Rotation(const float &d, const glm::vec3 &v) : degrees(d) , vector(v) {}
  Rotation() {}
};

class Translation {
 public:
  float distance;
  glm::vec3 vector;
  Translation(const float &d, const glm::vec3 &v) : distance(d) , vector(v) {}
};

void FillVertsAndIndsTriangles( std::vector<glm::vec3> &vertices
                     , std::vector<GLuint> &indices
                     , const Map &mp
                     ) {
  int xdim = mp._xdim;
  int ydim = mp._ydim;
  int cnt = 0;
  for ( int x = 0 ; x < xdim ; ++x ) {
    for ( int y = 0 ; y < ydim ; ++y ) {
      float xx1 = 1.5*float(x)/float(xdim)-.75;
      float yy1 = 1.5*float(y)/float(ydim)-.75;
      float xx2 = 1.5*float(x+1)/float(xdim)-.75;
      float yy2 = 1.5*float(y+1)/float(ydim)-.75;
      float zz = (mp._tiles[y*xdim + x].height)/100.;
      vertices.push_back(glm::vec3(xx1,yy1,zz));
      vertices.push_back(glm::vec3(xx1,yy2,zz));
      vertices.push_back(glm::vec3(xx2,yy2,zz));
      vertices.push_back(glm::vec3(xx2,yy1,zz));
      indices.push_back(cnt+0);
      indices.push_back(cnt+1);
      indices.push_back(cnt+2);
      indices.push_back(cnt+3);
      indices.push_back(cnt+0);
      indices.push_back(cnt+2);
      cnt += 4;
    }
  }
}

void FillVertsAndIndsLines( std::vector<glm::vec3> &vertices
                     , std::vector<GLuint> &indices
                     , const Map &mp
                     ) {
  int xdim = mp._xdim;
  int ydim = mp._ydim;
  int cnt = 0;
  for ( int x = 0 ; x < xdim ; ++x ) {
    for ( int y = 0 ; y < ydim ; ++y ) {
      float xx1 = 1.5*float(x)/float(xdim)-.75;
      float yy1 = 1.5*float(y)/float(ydim)-.75;
      float xx2 = 1.5*float(x+1)/float(xdim)-.75;
      float yy2 = 1.5*float(y+1)/float(ydim)-.75;
      float zz = (mp._tiles[y*xdim + x].height)/100.;
      vertices.push_back(glm::vec3(xx1,yy1,zz));
      vertices.push_back(glm::vec3(xx1,yy2,zz));
      vertices.push_back(glm::vec3(xx2,yy2,zz));
      vertices.push_back(glm::vec3(xx2,yy1,zz));
      indices.push_back(cnt+0);indices.push_back(cnt+1);
      indices.push_back(cnt+1);indices.push_back(cnt+2);
      indices.push_back(cnt+2);indices.push_back(cnt+3);
      indices.push_back(cnt+3);indices.push_back(cnt+0);
      cnt += 4;
    }
  }
}

class GlLayer {
 public:
  GLuint va; // Vertex array
  GLuint vb; // Vertex buffer
  GLuint ib; // Vertex array
  GLuint sp; // Shader program

  GLenum drawType; // Type of drawing (GL_POINTS, GL_LINE, etc.)

  GlLayer() {
    glGenVertexArraysOES(1,&this->va);
    glBindVertexArrayOES(this->va);

    glGenBuffers(1, &this->vb);
    glBindBuffer(GL_ARRAY_BUFFER, this->vb);

    glGenBuffers(1, &this->ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,this->ib);

    this->sp = GL_INVALID_VALUE;
  }

  void BindVB( std::vector<glm::vec3> &v , GLenum usage = GL_STATIC_DRAW ) {
    glBufferData(GL_ARRAY_BUFFER, v.size()*sizeof(glm::vec3), glm::value_ptr(v[0]), usage);
  }

  void BindIB( std::vector<GLuint> &i , GLenum usage = GL_STATIC_DRAW ) {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*i.size(), i.data(), usage);
  }

  void CompileShaderProgram( const char *vShader
                           , const char *fShader
                           , const std::vector<std::string> inputs
                           ) {
    this->sp = glCreateProgram();

    // Compile shaders
    if ( vShader != NULL ) {
      GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertexShader,1,&vShader,NULL);
      glCompileShader(vertexShader);
      shaderCompileCheck(vertexShader);
      glAttachShader(this->sp, vertexShader);
    }

    if ( fShader != NULL ) {
      GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragmentShader,1,&fShader,NULL);
      glCompileShader(fragmentShader);
      shaderCompileCheck(fragmentShader);
      glAttachShader(this->sp, fragmentShader);
    }

    glLinkProgram(this->sp);
    glUseProgram(this->sp);

    for ( auto s : inputs ) {
      GLint posAttrib = glGetAttribLocation(this->sp,s.c_str());
      glEnableVertexAttribArray(posAttrib);
      glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
  }

  void RotateView( const std::string &uniformName
                 , const std::vector<Rotation> &rotations
                 , const std::vector<Translation> &translations
                 ) {
      glm::mat4 view = glm::mat4(1.0f);
      for ( int i = 0 ; i < rotations.size() ; ++i ) {
        view = glm::rotate(view,glm::radians(rotations[i].degrees), rotations[i].vector);
      }
      GLint uniTrans = glGetUniformLocation(this->sp, uniformName.c_str());
      glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(view));
  }

  void UseThisLayer() {
    glBindVertexArrayOES(this->va);
    glUseProgram(this->sp);
  }


};

enum RotationAxes {
    ROTATE_X
  , ROTATE_Y
  , ROTATE_Z
  , ROTATE_TOTAL
};

void checkShaderCompile(GLuint s) {
  GLint success = 0;
  glGetShaderiv(s,GL_COMPILE_STATUS,&success);
  if(success == GL_FALSE)
  {
  	GLint maxLength = 0;
  	glGetShaderiv(s, GL_INFO_LOG_LENGTH, &maxLength);
  
  	// The maxLength includes the NULL character
  	std::vector<GLchar> errorLog(maxLength);
  	glGetShaderInfoLog(s, maxLength, &maxLength, &errorLog[0]);
  
  	// Provide the infolog in whatever manor you deem best.
  	// Exit with failure.
  	glDeleteShader(s); // Don't leak the shader.
  	return;
  }
}


int SDL_main(int argc, char **argv) {
  glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress ( "glGenVertexArraysOES" );
  glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress ( "glBindVertexArrayOES" );
  glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress ( "glDeleteVertexArraysOES" );
  glIsVertexArrayOES = (PFNGLISVERTEXARRAYOESPROC)eglGetProcAddress ( "glIsVertexArrayOES" );

  InitializeSDL("GLES3+SDL2 Tutorial",DISP_WIDTH,DISP_HEIGHT);

  glLineWidth(3.0f);
  glEnable(GL_DEPTH_TEST);

  // Because my depth seems to be backwards from what's expected
  glDepthFunc(GL_GREATER);
  glClearDepthf(0.f);

  std::vector<Rotation> rots(ROTATE_TOTAL);
  std::vector<Translation> trans;
  rots[ROTATE_X] = Rotation(-65.f,glm::vec3(1.f,0.f,0.f));
  rots[ROTATE_Y] = Rotation(  0.f,glm::vec3(0.f,1.f,0.f));
  rots[ROTATE_Z] = Rotation( 45.f,glm::vec3(0.f,0.f,1.f));


  std::vector<glm::vec3> verticesL, verticesT;
  std::vector<GLuint> indicesL, indicesT;
  //Map myMap = ReadMapFile("session/default/maps/example.map");
  Map myMap = GenerateMap(7,7,std::vector<VerticalityFeatures>(1,HILL));
  FillVertsAndIndsLines(     verticesL, indicesL , myMap );
  FillVertsAndIndsTriangles( verticesT, indicesT , myMap );

  GlLayer mapLayer;
  mapLayer.BindVB(verticesT);
  mapLayer.BindIB(indicesT);
  mapLayer.CompileShaderProgram( vertexSource
                                 , fragmentSource
                                 , std::vector<std::string>(1,"position")
                                 );

  glDrawElements(GL_TRIANGLES,indicesT.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
  mapLayer.RotateView( "view" , rots , trans);

  GlLayer gridLayer;
  gridLayer.BindVB(verticesL);
  gridLayer.BindIB(indicesL);
  gridLayer.CompileShaderProgram( vertexSource
                                 , fragmentSourceWhite
                                 , std::vector<std::string>(1,"position")
                                 );
  glDrawElements(GL_LINES,indicesL.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
  gridLayer.RotateView( "view" , rots , trans);

  int pos[2] = {3,3};
  std::vector<glm::vec3> posVerts(3);
  posVerts[0] = glm::vec3(pos[0],pos[1],0);
  posVerts[1] = glm::vec3(pos[0]+1,pos[1],0);
  posVerts[2] = glm::vec3(pos[0],pos[1]+1,0);

  GlLayer actorLayer;
  actorLayer.BindVB(posVerts);
  actorLayer.CompileShaderProgram( vertexSource
                                 , fragmentSourceWhite
                                 , std::vector<std::string>(1,"position")
                                 );
  glBufferData(GL_ARRAY_BUFFER, posVerts.size()*sizeof(glm::vec3),glm::value_ptr(posVerts[0]), GL_STATIC_DRAW);
  glDrawArrays(GL_TRIANGLES,0,3);
  actorLayer.RotateView( "view" , rots , trans);

  // Update the window
  SwapWindows();

  // Textures
  GLuint tex;
  glGenTextures(1,&tex);
  glBindTexture(GL_TEXTURE_2D,tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Wait for the user to quit
  bool quit = false;
  bool leftButtonDown = false;
  int xLast, yLast;
  while (!quit) {
    uint32_t frameStart = SDL_GetTicks();
    SDL_Event event;
    if (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT) {
        // User wants to quit
        quit = true;
      }
      else if ( event.type == SDL_KEYDOWN ) {
        switch (event.key.keysym.sym)
        {
          case SDLK_a:
            rots[ROTATE_Z].degrees -= 15.;
            break;
          case SDLK_d:
            rots[ROTATE_Z].degrees += 15.;
            break;
          case SDLK_s:
            rots[ROTATE_X].degrees -= 5.;
            break;
          case SDLK_w:
            rots[ROTATE_X].degrees += 5.;
            break;
          case SDLK_UP:
            pos[0] += 1;
            break;
          case SDLK_DOWN:
            pos[0] -= 1;
            break;
          case SDLK_LEFT:
            pos[1] += 1;
            break;
          case SDLK_RIGHT:
            pos[1] -= 1;
            break;
          case SDLK_q:
            quit = true;
            break;
          default:
            break;
        }
      }
      else if ( uint8_t(event.type) == uint8_t(SDL_MOUSEBUTTONDOWN) ) {
        switch(event.button.button)
        {
          case SDL_BUTTON_LEFT:
            //std::cout << "left click\n";
            SDL_GetMouseState(&xLast,&yLast);
            leftButtonDown = true;
            break;
          case SDL_BUTTON_RIGHT:
            std::cout << "right click\n";
            break;
          case SDL_BUTTON_MIDDLE:
            std::cout << "mid click\n";
            break;
          case SDL_BUTTON_X1:
            std::cout << "x1 click\n";
            break;
          case SDL_BUTTON_X2:
            std::cout << "x2 click\n";
            break;
          default:
            break;
        }
      }
      else if ( event.type == SDL_MOUSEBUTTONUP ) {
        switch(event.button.button)
        {
          case SDL_BUTTON_LEFT:
            //std::cout << "left up\n";
            SDL_GetMouseState(&xLast,&yLast);
            leftButtonDown = false;
            break;
          case SDL_BUTTON_RIGHT:
            std::cout << "right click\n";
            break;
          case SDL_BUTTON_MIDDLE:
            std::cout << "mid click\n";
            break;
          case SDL_BUTTON_X1:
            break;
          case SDL_BUTTON_X2:
            break;
          default:
            break;
        }
      }
    }
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //glClearDepthf(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if ( leftButtonDown ) {
      int xNow, yNow;
      uint32_t buttons = SDL_GetMouseState(&xNow,&yNow);
      rots[ROTATE_Z].degrees += float(xNow - xLast)/100.;
      rots[ROTATE_X].degrees += float(yNow - yLast)/100.;
      if ( buttons & SDL_BUTTON_LMASK == 0 ) {
        leftButtonDown = false;
      }
    }

    mapLayer.UseThisLayer();
    glDrawElements(GL_TRIANGLES,indicesT.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
    mapLayer.RotateView( "view" , rots , trans);

    gridLayer.UseThisLayer();
    glDrawElements(GL_LINES,indicesL.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
    gridLayer.RotateView( "view" , rots , trans);

    float x1 = 1.5*float(pos[0]  )/float(myMap._xdim)-.75;
    float x2 = 1.5*float(pos[0]+1)/float(myMap._xdim)-.75;
    float y1 = 1.5*float(pos[1]  )/float(myMap._ydim)-.75;
    float y2 = 1.5*float(pos[1]+1)/float(myMap._ydim)-.75;
    posVerts[0] = glm::vec3(x1,y1,myMap.h(pos[0],pos[1])/100.);
    posVerts[1] = glm::vec3(x2,y1,myMap.h(pos[0],pos[1])/100.);
    posVerts[2] = glm::vec3(x1,y2,(myMap.h(pos[0],pos[1])+10)/100.);
    glBufferData(GL_ARRAY_BUFFER, posVerts.size()*sizeof(glm::vec3),glm::value_ptr(posVerts[0]), GL_STATIC_DRAW);

    actorLayer.UseThisLayer();
    glDrawArrays(GL_TRIANGLES,0,3);
    actorLayer.RotateView( "view" , rots , trans );

    SwapWindows();

    //uint32_t frameEnd = SDL_GetTicks();
    //uint32_t framesUsed = frameEnd - frameStart;
    //printf("%d ms\n", framesUsed);

    //int cnt = 0;
    //if ( framesUsed < TARGET_FRAME_TIME ) {
    //  //printf(".");
    //  SDL_Delay(TARGET_FRAME_TIME - framesUsed);
    //}
    //if ( cnt > 20 ) printf("\n");
    //cnt = 0;
  }
  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  return SDL_main(argc,argv);
}
