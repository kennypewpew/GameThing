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
    uniform vec3 incolor;
    
    void main()
    {
        fragColor = vec4(incolor, 1.0);
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

void GenerateIndicesLines( std::vector<GLuint> &indices , int n ) {
  for ( int i = 0 ; i < n ; ++i ) {
    indices.push_back(4*i+0);indices.push_back(4*i+1);
    indices.push_back(4*i+1);indices.push_back(4*i+2);
    indices.push_back(4*i+2);indices.push_back(4*i+3);
    indices.push_back(4*i+3);indices.push_back(4*i+0);
  }
}

void AddIndexTriangle( std::vector<GLuint> &i
                     , const int &v1
                     , const int &v2
                     , const int &v3
                     ) {
  i.push_back(v1);
  i.push_back(v2);
  i.push_back(v3);
}

void GenerateIndicesTriangles( std::vector<GLuint> &indices , int n ) {
  for ( int i = 0 ; i < n ; ++i ) {
    AddIndexTriangle(indices, 4*i , 4*i + 1 , 4*i + 2 );
    AddIndexTriangle(indices, 4*i , 4*i + 2 , 4*i + 3 );
  }
}

float CoordToValue(const int &c, const int &max) {
  float factor = 1.5;
  return factor*float(c)/float(max) - factor/2.;
}

float HeightToValue( const int &h ) {
  float factor = 100.;
  return float(h) / factor;
}

void AddSquareTile( std::vector<glm::vec3> &vertices
                  , const int &x
                  , const int &xdim
                  , const int &y
                  , const int &ydim
                  , const int &z
                  ) {
  float xx1 = CoordToValue( x   , xdim );
  float yy1 = CoordToValue( y   , ydim );
  float xx2 = CoordToValue( x+1 , xdim );
  float yy2 = CoordToValue( y+1 , ydim );
  float zz = HeightToValue(z);
  vertices.push_back(glm::vec3(xx1,yy1,zz));
  vertices.push_back(glm::vec3(xx1,yy2,zz));
  vertices.push_back(glm::vec3(xx2,yy2,zz));
  vertices.push_back(glm::vec3(xx2,yy1,zz));
}

void FillVertsAndInds( std::vector<glm::vec3> &vertices
                     , std::vector<GLuint> &indices
                     , const Map &mp
                     , const GLenum &type
                     ) {
  int xdim = mp._xdim;
  int ydim = mp._ydim;
  for ( int y = 0 ; y < ydim ; ++y ) {
    for ( int x = 0 ; x < xdim ; ++x ) {
      AddSquareTile( vertices , x , xdim , y , ydim , mp.h(x,y) );
    }
  }
  for ( size_t i = 0 ; i < mp._extras.size() ; ++i ) {
    int x = mp._extras[i].x;
    int y = mp._extras[i].y;
    int h = mp._extras[i].height;
    AddSquareTile( vertices , x , xdim , y , ydim , h );
  }
  if ( type == GL_LINES     ) GenerateIndicesLines    ( indices , xdim*ydim + mp._extras.size() );
  if ( type == GL_TRIANGLES ) GenerateIndicesTriangles( indices , xdim*ydim + mp._extras.size() );
}

bool CoordInVector( const Coord2D &c , const std::vector<Coord2D> &v ) {
  for ( size_t i = 0 ; i < v.size() ; ++i ) {
    if ( c.x == v[i].x && c.y == v[i].y ) return true;
  }
  return false;
}

void FillWallIndices( std::vector<GLuint> &inds , const Map &mp ) {
  int xdim = mp._xdim;
  int ydim = mp._ydim;

  // Check for openings
  std::vector<Coord2D> xskip;
  std::vector<Coord2D> yskip;
  for ( size_t i = 0 ; i < mp._extras.size() ; ++i ) {
    for ( size_t j = 0 ; j < mp._extras[i].connections.size() ; ++j ) {
      int xdiff = mp._extras[i].connections[j].x - mp._extras[i].x;
      int ydiff = mp._extras[i].connections[j].y - mp._extras[i].y;
      if      ( xdiff > 0 ) xskip.push_back( (Coord2D){ mp._extras[i].x   , mp._extras[i].y   } );
      else if ( xdiff < 0 ) xskip.push_back( (Coord2D){ mp._extras[i].x-1 , mp._extras[i].y   } );
      else if ( ydiff > 0 ) yskip.push_back( (Coord2D){ mp._extras[i].x   , mp._extras[i].y   } );
      else if ( ydiff < 0 ) yskip.push_back( (Coord2D){ mp._extras[i].x   , mp._extras[i].y-1 } );

      int cnt1 = 4*mp._extras[i].connections[j].x + 4*xdim*mp._extras[i].connections[j].y;
      int cnt2 = 4*i + 4*xdim*ydim;
      int a1, a2, b1, b2;
      if ( xdiff > 0 ) {
        a1 = cnt1 + 0;
        a2 = cnt1 + 1;
        b1 = cnt2 + 2;
        b2 = cnt2 + 3;
      }
      else if ( xdiff < 0 ) {
        a1 = cnt1 + 2;
        a2 = cnt1 + 3;
        b1 = cnt2 + 0;
        b2 = cnt2 + 1;
      }
      else if ( ydiff > 0 ) {
        a1 = cnt1 + 3;
        a2 = cnt1 + 0;
        b1 = cnt2 + 1;
        b2 = cnt2 + 2;
      }
      else if ( ydiff < 0 ) {
        a1 = cnt1 + 1;
        a2 = cnt1 + 2;
        b1 = cnt2 + 3;
        b2 = cnt2 + 0;
      }
      AddIndexTriangle( inds , a1 , a2 , b1 );
      AddIndexTriangle( inds , a1 , b2 , b1 );
    }
  }

  // y direction walls
  for ( int y = 0 ; y < ydim-1 ; ++y ) {
    for ( int x = 0 ; x < xdim ; ++x ) {
      if ( CoordInVector( (Coord2D){ x , y } , yskip ) ) continue;
      int cnt = 4*x + 4*xdim*y;
      int i_01 = cnt + 1;
      int i_11 = cnt + 2;
      int k_00 = cnt + 4*xdim;
      int k_10 = cnt + 4*xdim + 3;
      AddIndexTriangle( inds , i_01 , i_11, k_00 );
      AddIndexTriangle( inds , i_11 , k_00, k_10 );
    }
  }
  // x direction walls
  for ( int y = 0 ; y < ydim ; ++y ) {
    for ( int x = 0 ; x < xdim-1 ; ++x ) {
      if ( CoordInVector( (Coord2D){ x , y } , xskip ) ) continue;
      int cnt = 4*x + 4*xdim*y;
      int i_11 = cnt + 2;
      int i_10 = cnt + 3;
      int j_00 = cnt + 4 + 0;
      int j_01 = cnt + 4 + 1;
      AddIndexTriangle( inds , i_10 , i_11, j_00 );
      AddIndexTriangle( inds , i_11 , j_01, j_00 );
    }
  }
}


class GlLayer {
 public:
  GLuint va; // Vertex array
  GLuint vb; // Vertex buffer
  GLuint ib; // Vertex array
  GLuint sp; // Shader program
  GLuint color; // Color uniform location

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
    this->color = glGetUniformLocation(this->sp, "incolor");
  }

  void RotateView( const std::string &uniformName
                 , const std::vector<Rotation> &rotations
                 , const std::vector<Translation> &translations
                 ) {
      glm::mat4 view = glm::mat4(1.0f);
      for ( size_t i = 0 ; i < rotations.size() ; ++i ) {
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

  glLineWidth(5.0f);
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
  std::vector<GLuint> indicesL, indicesT, indicesW;
  //Map myMap = ReadMapFile("session/default/maps/example.map");
  Map myMap = GenerateMap(7,7,std::vector<VerticalityFeatures>(1,MOUNTAIN));
  FillVertsAndInds( verticesL, indicesL , myMap , GL_LINES );
  FillVertsAndInds( verticesT, indicesT , myMap , GL_TRIANGLES );
  FillWallIndices( indicesW , myMap );

  GlLayer mapLayer;
  mapLayer.BindVB(verticesT);
  mapLayer.BindIB(indicesT);
  mapLayer.CompileShaderProgram( vertexSource
                                 , fragmentSource
                                 , std::vector<std::string>(1,"position")
                                 );

  glDrawElements(GL_TRIANGLES,indicesT.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
  mapLayer.RotateView( "view" , rots , trans);

  GlLayer mapWallLayer;
  mapWallLayer.BindVB(verticesT);
  mapWallLayer.BindIB(indicesW);
  mapWallLayer.CompileShaderProgram( vertexSource
                                 , fragmentSource
                                 , std::vector<std::string>(1,"position")
                                 );

  glDrawElements(GL_TRIANGLES,indicesW.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
  mapWallLayer.RotateView( "view" , rots , trans);

  GlLayer gridLayer;
  gridLayer.BindVB(verticesL);
  gridLayer.BindIB(indicesL);
  gridLayer.CompileShaderProgram( vertexSource
                                 , fragmentSource
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

float pixels[] = {
    0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
};
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels);

  // Wait for the user to quit
  bool quit = false;
  bool leftButtonDown = false;
  int xLast, yLast;
  while (!quit) {
    //uint32_t frameStart = SDL_GetTicks();
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
      rots[ROTATE_Z].degrees += float(xNow - xLast)/2.;
      rots[ROTATE_X].degrees += float(yNow - yLast)/10.;
      if ( buttons & (SDL_BUTTON_LMASK == 0) ) {
        leftButtonDown = false;
      }
      xLast = xNow;
      yLast = yNow;
    }

    mapLayer.UseThisLayer();
    glUniform3f(mapLayer.color, 1.0f, 0.0f, 1.0f);
    glDrawElements(GL_TRIANGLES,indicesT.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
    mapLayer.RotateView( "view" , rots , trans);

    mapWallLayer.UseThisLayer();
    glUniform3f(mapWallLayer.color, 0.8f, 0.0f, 0.8f);
    glDrawElements(GL_TRIANGLES,indicesW.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
    mapWallLayer.RotateView( "view" , rots , trans);

    gridLayer.UseThisLayer();
    glUniform3f(gridLayer.color, 1.0f, 1.0f, 1.0f);
    glDrawElements(GL_LINES,indicesL.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
    gridLayer.RotateView( "view" , rots , trans);

    float x1 = CoordToValue( pos[0]   , myMap._xdim );
    float x2 = CoordToValue( pos[0]+1 , myMap._xdim );
    float y1 = CoordToValue( pos[1]   , myMap._xdim );
    float y2 = CoordToValue( pos[1]+1 , myMap._xdim );
    posVerts[0] = glm::vec3( x1 , y1 , HeightToValue( myMap.h(pos[0],pos[1])    ) );
    posVerts[1] = glm::vec3( x2 , y1 , HeightToValue( myMap.h(pos[0],pos[1])    ) );
    posVerts[2] = glm::vec3( x1 , y2 , HeightToValue( myMap.h(pos[0],pos[1])+10 ) );
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
