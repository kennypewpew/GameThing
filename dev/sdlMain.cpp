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
#include <cmath>
#include <map>

#include "Maps.h"
#include "MapGen.h"
#include "Window.h"
#include "GlLayer.h"
#include "Types.h"
#include "Actor.h"
#include "Physics.h"

#include "stb_image.h"

#ifdef __ANDROID__
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
//#include "JNIHelper.h"
#endif

const unsigned int DISP_WIDTH = 480;
const unsigned int DISP_HEIGHT = 960;
const int HEIGHT_INCREMENT = 4;

const unsigned int TARGET_FPS = 1;
const unsigned int TARGET_FRAME_TIME = 1000 / TARGET_FPS;

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

// Coordinates are 1-indexed instead of 0-indexed so out of bounds doesn't return (0,0)
int ColorToCoord(const GLubyte &rawColor, const int &maxDim) {
  return ( rawColor / ( 255/(maxDim+1) ) ) - 1;
}

float CoordToColor(const int &coord , const int &maxDim) {
  return float(coord+1)/float(maxDim+1);
}

void AddSquareTileColorID( std::vector<glm::vec3> &colors
                  , const int &x
                  , const int &y
                  , const int &z
                  , const int &xdim
                  , const int &ydim
                  , const int &zdim
                  ) {
  float xx1 = CoordToColor(x,xdim);
  float yy1 = CoordToColor(y,ydim);
  float zz1 = CoordToColor(z,zdim);
  for ( int i = 0 ; i < 4 ; ++i ) {
    colors.push_back(glm::vec3(xx1,yy1,zz1));
  }
}

void AddSquareTileTextureCoords( std::vector<glm::vec2> &textureCoords ) {
  textureCoords.push_back(glm::vec2(0.,0.));
  textureCoords.push_back(glm::vec2(0.,1.));
  textureCoords.push_back(glm::vec2(1.,1.));
  textureCoords.push_back(glm::vec2(1.,0.));
}


void AddSquareTile( std::vector<glm::vec3> &vertices
                  , const int &x
                  , const int &xdim
                  , const int &y
                  , const int &ydim
                  , const float &z
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

void GetTileCoords( const GLuint &tileBuffer, const Map &map , int *outCoords ) {
  int xRightClick, yRightClick;
  SDL_GetMouseState(&xRightClick,&yRightClick);
  glBindFramebuffer(GL_FRAMEBUFFER,tileBuffer);
  GLubyte data[4];
  glReadPixels(xRightClick,DISP_HEIGHT-yRightClick,1,1,GL_RGBA,GL_UNSIGNED_BYTE,data);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  int x = ColorToCoord(data[0],map._xdim);
  int y = ColorToCoord(data[1],map._ydim);
  int e = ColorToCoord(data[2],map._extras.size())-1;
  if ( x >= 0 && x < map._xdim && y >= 0 && y < map._ydim ) {
    outCoords[0] = x;
    outCoords[1] = y;
    if ( e > -1 ) outCoords[2] = map._extras[e].height;
    else          outCoords[2] = map.h(x,y);
    outCoords[3] = e;
  }
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

class BattleMap;
bool QuitMap(const SDL_Event &e , BattleMap* mp);

class BattleMap {
 public:
    std::vector<Rotation> rots;
    std::vector<Translation> trans;
    std::vector<glm::vec3> verticesGridLines, verticesTileTriangles;
    std::vector<glm::vec3> verticesTileTrianglesUp;
    std::vector<GLuint> indicesGridLines, indicesTriangles, indicesTileWalls;
    std::vector<glm::vec3> colorIDs;
    std::vector<glm::vec2> verticesTexture;
    std::vector<glm::vec2> verticesTextureWalls;
    GLuint coordinateBuffer;
    ActorTetrahedron tet1;
    bool quit;
    bool leftButtonDown;
    int xLast, yLast;
    float magnification = 0.8;
    glm::mat4 zoomLevel;
    glm::mat4 shiftMatrix;
    glm::mat4 view;
    glm::vec3 shiftVector;
    Map myMap, bufferMap;
    std::vector<glm::vec3> verticesMoveOverlay;
    std::vector<GLuint> indicesMoveOverlay;
    std::vector<glm::vec3> verticesTrajectory;
    std::vector<GLuint> indicesTrajectory;
    int endTraj[3];

  BattleMap() {

    InitializeSDL("GLES3+SDL2 Tutorial",DISP_WIDTH,DISP_HEIGHT);

    glEnable(GL_DEPTH_TEST);

    this->rots = std::vector<Rotation>(ROTATE_TOTAL);
    this->rots[ROTATE_X] = Rotation(-75.f,glm::vec3(1.f,0.f,0.f));
    this->rots[ROTATE_Y] = Rotation(  0.f,glm::vec3(0.f,1.f,0.f));
    this->rots[ROTATE_Z] = Rotation( 45.f,glm::vec3(0.f,0.f,1.f));

    this->zoomLevel = glm::scale(glm::mat4(1.f),glm::vec3(this->magnification,this->magnification,0.1));

    this->shiftVector = glm::vec3(0.0f,0.f,0.f);
    this->shiftMatrix = glm::translate(glm::mat4(1.f), this->shiftVector);

    // Because my depth seems to be backwards from what's expected
    glDepthFunc(GL_GREATER);
    glClearDepthf(0.f);

    this->quit = false;
    this->leftButtonDown = false;
  }

  void CreateTileRenderBuffer() {
    glGenFramebuffers(1,&this->coordinateBuffer);
    unsigned int rbo[2];
    glGenRenderbuffers(2, rbo);
    glBindFramebuffer(GL_FRAMEBUFFER,this->coordinateBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, DISP_WIDTH, DISP_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo[0]);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, DISP_WIDTH, DISP_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo[1]);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cout << "Failed to bind tile coordinate buffer\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);
  }

  void UpdateViewTransformation() {
    //mapTileLayer.RotateView( "view" , this->rots , this->trans);
    this->view = this->zoomLevel;
    for ( size_t i = 0 ; i < this->rots.size() ; ++i ) {
      this->view = glm::rotate(this->view,glm::radians(this->rots[i].degrees), this->rots[i].vector);
    }
    for ( size_t i = 0 ; i < this->trans.size() ; ++i ) {
      this->view = glm::translate(this->view,this->trans[i].vector);
    }
    this->view *= this->shiftMatrix;

    //glm::vec3 camera = glm::vec3(0.,0.1*this->magnification,0.5*this->magnification);
    //glm::vec3 center = glm::vec3(0.,0.,0.);
    //glm::vec3 upDir = glm::vec3(0.5,0.5,0.2);
    //this->view = glm::lookAt(camera,center,upDir);

    ////mapTileLayer.SetUniform( "zoom" , this->zoomLevel );
    //GLint uniTrans = glGetUniformLocation(this->sp, uniformName.c_str());
    //glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(zoom));

    ////mapTileLayer.SetUniform( "shift" , this->shiftMatrix );
    //GLint uniTrans = glGetUniformLocation(this->sp, uniformName.c_str());
    //glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(shift));

// zoom * view * shift
  }


  void DrawTileIdLayer(GlLayer &mapLayer) {
    glBindFramebuffer(GL_FRAMEBUFFER,this->coordinateBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mapLayer.UseThisLayer();
    mapLayer.SetUniform( "view" , this->view );
    glDrawElements(GL_TRIANGLES,this->indicesTriangles.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void DrawMapOverlay(GlLayer &mapTileLayer) {
    mapTileLayer.UseThisLayer();
    glm::vec4 color(0.0f, 0.0f, 1.0f, 0.5f);
    mapTileLayer.SetUniform( "incolor" , color );
    mapTileLayer.SetUniform( "view" , this->view );
    mapTileLayer.BindCopyVB( this->verticesMoveOverlay , 3 );
    mapTileLayer.BindCopyIB( this->indicesMoveOverlay );
    glDrawElements(GL_TRIANGLES,this->indicesMoveOverlay.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
  }

  void DrawMapTiles(GlLayer &mapTileLayer) {
    mapTileLayer.UseThisLayer();
    glm::vec4 color(1.0f, 0.0f, 1.0f, 1.f);
    mapTileLayer.SetUniform( "incolor" , color );
    mapTileLayer.SetUniform( "view" , this->view );
    glDrawElements(GL_TRIANGLES,this->indicesTriangles.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
  }

  void DrawMapWalls(GlLayer &mapWallLayer) {
    mapWallLayer.UseThisLayer();
    glm::vec4 color(0.8f, 0.0f, 0.8f, 0.9f);
    mapWallLayer.SetUniform( "incolor" , color );
    mapWallLayer.SetUniform( "view" , this->view );
    glDrawElements(GL_TRIANGLES,this->indicesTileWalls.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
  }

  void DrawGridLines(GlLayer &gridLayer) {
    glLineWidth(5.0f);
    gridLayer.UseThisLayer();
    glm::vec4 color(1.0f, 1.0f, 1.0f, 1.f);
    gridLayer.SetUniform( "incolor" , color );
    gridLayer.SetUniform( "view" , this->view );
    glDrawElements(GL_LINES,this->indicesGridLines.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
    glLineWidth(1.0f);
  }

  void DrawTrajectory(GlLayer &gridLayer) {
    glLineWidth(5.0f);
    gridLayer.UseThisLayer();
    glm::vec4 color(0.0f, 1.0f, 0.0f, 1.f);
    gridLayer.SetUniform( "incolor" , color );
    gridLayer.SetUniform( "view" , this->view );
    gridLayer.BindCopyVB( this->verticesTrajectory , 3 );
    gridLayer.BindCopyIB( this->indicesTrajectory );
    glDrawElements(GL_LINES,this->indicesGridLines.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
    glLineWidth(1.0f);
  }

  void DrawActor(GlLayer &actorLayer , const std::vector<Translation> &shift) {
    actorLayer.UseThisLayer();
    glm::vec4 color(0.5f, 0.5f, 0.5f, 1.f);
    actorLayer.SetUniform( "incolor" , color );
    glBufferData(GL_ARRAY_BUFFER, tet1.posVerts.size()*sizeof(glm::vec3),glm::value_ptr(tet1.posVerts[0]), GL_STATIC_DRAW);
    actorLayer.RotateView( "view" , this->rots , shift );
    actorLayer.SetUniform( "zoom" , this->zoomLevel );
    actorLayer.SetUniform( "shift" , this->shiftMatrix );
    glDrawArrays(GL_TRIANGLES,0,9);
    actorLayer.SetUniform( "zoom" , glm::mat4(1.) );
  }

  void FillColorIDs( const Map &myMap ) {
    for ( int y = 0 ; y < this->myMap._ydim ; ++y ) {
      for ( int x = 0 ; x < this->myMap._xdim ; ++x ) {
	AddSquareTileColorID( this->colorIDs , x , y , 0 , this->myMap._xdim , this->myMap._ydim , this->myMap._extras.size() );
      }
    }
    for ( size_t i = 0 ; i < this->myMap._extras.size() ; ++i ) {
      ExtraMapTile e = this->myMap._extras[i];
      AddSquareTileColorID( this->colorIDs , e.x , e.y , i+1 , this->myMap._xdim , this->myMap._ydim , this->myMap._extras.size() );
    }
  }

  void ShiftMap( const float &dist , const int &rotate ) {
    this->shiftVector += dist * glm::vec3(
          glm::sin(glm::radians(rots[ROTATE_Z].degrees + rotate))
        , glm::cos(glm::radians(rots[ROTATE_Z].degrees + rotate))
        , 0.
        );
    this->shiftMatrix = glm::translate(glm::mat4(1.f), this->shiftVector);
  }

  void Zoom( const float &z ) {
    this->magnification += z;
    if ( this->magnification < 0. ) this->magnification = 0.;
    if ( this->magnification > 3. ) this->magnification = 3.;
    this->zoomLevel = glm::scale(glm::mat4(1.f),glm::vec3(this->magnification,this->magnification,0.1));
  }

  void SpinMap( const float &deg ) {
    this->rots[ROTATE_Z].degrees += deg;
    if ( this->rots[ROTATE_Z].degrees < 0.)   this->rots[ROTATE_Z].degrees += 360.;
    if ( this->rots[ROTATE_Z].degrees > 360.) this->rots[ROTATE_Z].degrees -= 360.;
  }

  void TiltMap( const float &deg ) {
    this->rots[ROTATE_X].degrees += deg;
    if ( this->rots[ROTATE_X].degrees < -90.) this->rots[ROTATE_X].degrees = -90.;
    if ( this->rots[ROTATE_X].degrees >   0.) this->rots[ROTATE_X].degrees = 0.;
  }

  void HandleInputEvent( const SDL_Event &e , const Map &map , std::map<Uint32,bool (*)(const SDL_Event&,BattleMap*)> &handlers ) {
      if ( handlers.count(e.type) ) (handlers[e.type])(e,this);
      if ( e.type == SDL_KEYDOWN ) {
        switch (e.key.keysym.sym)
        {
          case SDLK_a:
            SpinMap( -15. );
            break;
          case SDLK_d:
            SpinMap( +15. );
            break;
          case SDLK_s:
            TiltMap( -5. );
            break;
          case SDLK_w:
            TiltMap( +5. );
            break;
          case SDLK_UP:
            ShiftMap( +0.05f , 0 );
            break;
          case SDLK_DOWN:
            ShiftMap( -0.05f , 0 );
            break;
          case SDLK_LEFT:
            ShiftMap( -0.05f , 90 );
            break;
          case SDLK_RIGHT:
            ShiftMap( +0.05f , 90 );
            break;
          case SDLK_y:
            Zoom(+0.05f);
            break;
          case SDLK_t:
            Zoom(-0.05f);
            break;
          case SDLK_q:
            this->quit = true;
            break;
          default:
            break;
        }
      }
      else if ( uint8_t(e.type) == uint8_t(SDL_MOUSEBUTTONDOWN) ) {
        switch(e.button.button)
        {
          case SDL_BUTTON_LEFT:
            SDL_GetMouseState(&this->xLast,&this->yLast);
            this->leftButtonDown = true;
            break;
          case SDL_BUTTON_RIGHT:
            {
              //GetTileCoords( coordinateBuffer, map , tet1.pos );
              GetTileCoords( coordinateBuffer, map , this->endTraj );
              break;
            }
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
      else if ( e.type == SDL_MOUSEBUTTONUP ) {
        switch(e.button.button)
        {
          case SDL_BUTTON_LEFT:
            //SDL_GetMouseState(&this->xLast,&this->yLast);
            this->leftButtonDown = false;
            break;
            //case SDL_BUTTON_RIGHT:
            //  std::cout << "right click\n";
            //  break;
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
      else if ( e.type == SDL_MOUSEWHEEL ) {
        if (e.wheel.y > 0 || e.mgesture.dDist > 0 ) {
          Zoom(+0.05f);
        }
        else if (e.wheel.y < 0 ) {
          Zoom(-0.05f);
        }
      }
      else if ( e.type == SDL_MULTIGESTURE ) {
        if ( e.mgesture.dDist > 0 ) {
          Zoom( e.mgesture.dDist/50. );
        }
        else if ( e.mgesture.dDist < 0 ) {
          Zoom( e.mgesture.dDist/50. );
        }
      }
    }

bool HeightReachable( const int &z1 , const int &z2 , const int &hmax ) {
  int diff = z1 - z2;
  if ( diff > 0 ) {
    return (abs(diff) <= hmax+HEIGHT_INCREMENT);
  }
  else {
    return (abs(diff) <= hmax);
  }
}

bool TileUpdate( const Map &mp , Map &buffer , const int &x1 , const int &y1 , const int &x2 , const int &y2 , const int &maxDiff , const int &e1 = -1,  const int &e2 = -1 ) {
  Coord3D c1,c2;
  int b1,b2;
  if ( e1 < 0 ) {
    c1.x = x1; c1.y = y1; c1.z = mp.h(x1,y1);
    b1 = buffer.h(x1,y1);
  }
  else {
    c1.x = mp.x(e1);
    c1.y = mp.y(e1);
    c1.z = mp.h(e1);
    b1 = buffer.h(e1);
  }

  if ( e2 < 0 ) {
    c2.x = x2; c2.y = y2; c2.z = mp.h(x2,y2);
    b2 = buffer.h(x2,y2);
  }
  else {
    c2.x = mp.x(e2);
    c2.y = mp.y(e2);
    c2.z = mp.h(e2);
    b2 = buffer.h(e2);
  }

  if ( e2 < 0 ) if ( !buffer.InBounds( c2.x , c2.y) ) return false;
  int less = b1 - 1;
  if ( b2 >= less  ) return false;
  if ( !HeightReachable( c1.z , c2.z , maxDiff ) ) return false;

  if ( e2 < 0 ) buffer.h(x2,y2) = less;
  else buffer._extras[e2].height = less;

  return true;
}

bool SetTileReachability( const Map &mp , Map &buffer , const int &j , const int &i , const int &maxDiff ) {
  bool todo = false;
  if ( buffer.h(j,i) ) {
    if ( TileUpdate(mp,buffer,j,i,j+1,i+0,maxDiff) ) todo = true;
    if ( TileUpdate(mp,buffer,j,i,j-1,i+0,maxDiff) ) todo = true;
    if ( TileUpdate(mp,buffer,j,i,j-0,i+1,maxDiff) ) todo = true;
    if ( TileUpdate(mp,buffer,j,i,j+0,i-1,maxDiff) ) todo = true;
  }
  return todo;
}

void SetReachable( const Map &mp , Map &buffer , const int &x , const int &y , const int &range , const int& maxDiff , const int &e = -1 ) {
  for ( int i = 0 ; i < mp._ydim * mp._xdim ; ++i ) {
    buffer._tiles[i].height = 0;
  }
  for ( size_t i = 0 ; i < mp._extras.size() ; ++i ) {
    buffer._extras[i].height = 0;
  }

  if ( e < 0 ) buffer.h(x,y) = range+1;
  else         buffer._extras[e].height = range+1;
  bool todo = true;
  while ( todo ) {
    todo = false;
    for ( int i = 0 ; i < mp._ydim ; ++i ) {
      for ( int j = 0 ; j < mp._xdim ; ++j ) {
        if ( buffer.h(j,i) ) {
          todo |= SetTileReachability(mp,buffer,j,i,maxDiff);
        }
      }
    }
    for ( size_t i = 0 ; i < mp._extras.size() ; ++i ) {
      if ( !buffer.h(i) ) {
        for ( size_t j = 0 ; j < mp._extras[i].connections.size() ; ++j ) {
          Coord2D k = mp._extras[i].connections[j];
          //bool r = HeightReachable( mp.h(k.x,k.y) , mp.h(i) , maxDiff );
          if ( buffer.h(k.x,k.y) )
            todo |= TileUpdate( mp , buffer , k.x , k.y , mp.x(i) , mp.y(i) , maxDiff , -1, i );
        }
      }
      else {
        //int less = buffer._extras[i].height - 1;
        for ( size_t j = 0 ; j < mp._extras[i].extra_connections.size() ; ++j ) {
          int k = mp._extras[i].extra_connections[j];
          todo |= TileUpdate( mp , buffer , mp.x(i) , mp.y(i) , mp.x(k) , mp.y(k) , maxDiff , i, k );
        }
        for ( size_t j = 0 ; j < mp._extras[i].connections.size() ; ++j ) {
          Coord2D k = mp._extras[i].connections[j];
          todo |= TileUpdate( mp , buffer , mp.x(i) , mp.y(i) , k.x , k.y , maxDiff , i, -i );
        }
      }
    }
  }
}

  void FillReachableVertsInds( const Map &mp , const Map &info , std::vector<glm::vec3> &verts , std::vector<GLuint> &inds ) {
    for ( int i = 0 ; i < mp._ydim ; ++i ) {
      for ( int j = 0 ; j < mp._xdim ; ++j ) {
        if ( info.h(j,i) ) {
          AddSquareTile( verts , j , mp._xdim , i , mp._ydim , float(mp.h(j,i))+0.02 );
        }
      }
    }
    for ( size_t i = 0 ; i < mp._extras.size() ; ++i ) {
      if ( info._extras[i].height ) {
        int xx = mp._extras[i].x;
        int yy = mp._extras[i].y;
        int hh = mp._extras[i].height;
        AddSquareTile( verts , xx , mp._xdim , yy , mp._ydim , float(hh)+0.02 );
      }
    }
    GenerateIndicesTriangles( inds , verts.size() );
  }

  void UpdateTrajectory( int *startPos , int *endPos , const float &vel ) {
      this->verticesTrajectory.clear();
      this->indicesTrajectory.clear();

      float dist = CoordsToDistance( startPos[0] , startPos[1] , endPos[0] , endPos[1] );
      float angle = MaxAngleToReach( dist , startPos[2] - endPos[2] , vel );
      if ( glm::isnan(angle) ) return;
//std::cout << angle << "\n";

      glm::vec3 posTraj = glm::vec3(CoordToValue(startPos[0]+0.5,myMap._xdim),CoordToValue(startPos[1]+0.5,myMap._ydim),HeightToValue(startPos[2]+5));
      verticesTrajectory.push_back(glm::vec3(posTraj));
      int steps = 10;
      float xdiff = (CoordToValue(endPos[0] , this->myMap._xdim ) - CoordToValue(startPos[0] , this->myMap._xdim)) / float(steps);
      float ydiff = (CoordToValue(endPos[1] , this->myMap._xdim ) - CoordToValue(startPos[1] , this->myMap._ydim)) / float(steps);
      float rdiff = dist / float(steps);
      glm::vec3 flatDisplacement = glm::vec3( xdiff , ydiff , 0 );
      for ( int i = 0 ; i < steps ; ++i ) {
        posTraj += flatDisplacement;
        float y = HeightToValue( HeightAtDistance( float(i+1)*rdiff , vel , angle , startPos[2]+5 ) );
        posTraj.z = y;
        this->verticesTrajectory.push_back(posTraj);
        this->indicesTrajectory.push_back(i);
        this->indicesTrajectory.push_back(i+1);
      }
  }

  int MainLoop(int argc, char **argv) {
    Shader shaderInputColor( std::string("shader/inputColor.vs") , std::string("shader/inputColor.fs") );
    Shader shaderSolidColor( std::string("shader/solidColor.vs") , std::string("shader/solidColor.fs") );
    Shader shaderTexture( std::string("shader/inputTexture.vs") , std::string("shader/inputTexture.fs") );

    //this->myMap = ReadMapFile("session/default/maps/example.map");
    this->myMap = GenerateMap(11,11,std::vector<VerticalityFeatures>(1,MOUNTAIN));
    //this->myMap = GenerateMap(7,7,std::vector<VerticalityFeatures>(1,MOUNTAIN));
    //this->myMap = GenerateMap(7,7,std::vector<VerticalityFeatures>(1,FLAT));
    this->myMap.h(5,5) = 20;
    FillVertsAndInds( this->verticesGridLines, this->indicesGridLines , this->myMap , GL_LINES );
    FillVertsAndInds( this->verticesTileTriangles , this->indicesTriangles , this->myMap , GL_TRIANGLES );
    FillWallIndices( this->indicesTileWalls , this->myMap );
    FillColorIDs( this->myMap );
    verticesTileTrianglesUp.resize(verticesTileTriangles.size());
    for ( size_t i = 0 ; i < verticesTileTriangles.size() ; ++i ) {
      this->verticesTileTrianglesUp[i] = glm::vec3(0,0,0.01) + this->verticesTileTriangles[i];
    }
    //PrintMap(this->myMap);

    // Create a buffer for knowing which tiles get clicked on
    CreateTileRenderBuffer();

    // Colored tile IDs for clicking on tiles
    GlLayer mapIdLayer( shaderInputColor );
    mapIdLayer.AddInput( "position" , 3 );
    mapIdLayer.AddInput( "incolor" , 3 );
    mapIdLayer.SetUniform( "zoom" , glm::mat4(1.) );
    mapIdLayer.SetUniform( "shift" , glm::mat4(1.) );
    mapIdLayer.BindCopyVB(this->verticesTileTriangles,3,0);
    mapIdLayer.BindCopyVB(this->colorIDs,3,1);
    mapIdLayer.BindCopyIB(this->indicesTriangles);

    // Flat parts of the tiles
    GlLayer mapTileLayer( shaderTexture );
    mapTileLayer.AddInput( "position" , 3 );
    mapTileLayer.SetUniform( "zoom" , glm::mat4(1.) );
    mapTileLayer.SetUniform( "shift" , glm::mat4(1.) );
    mapTileLayer.BindCopyVB(this->verticesTileTriangles,3);
    mapTileLayer.BindCopyIB(this->indicesTriangles);

   // Movement range overlay
    GlLayer mapTileOverlay( shaderSolidColor );
    mapTileOverlay.AddInput( "position" , 3 );
    mapTileOverlay.SetUniform( "zoom" , glm::mat4(1.) );
    mapTileOverlay.SetUniform( "shift" , glm::mat4(1.) );
    mapTileOverlay.BindCopyVB(this->verticesTileTrianglesUp,3);
    mapTileOverlay.BindCopyIB(this->indicesTriangles);

    // Walls connecting adjacent tiles on different z-levels
    GlLayer mapWallLayer( shaderTexture );
    mapWallLayer.AddInput( "position" , 3 );
    mapWallLayer.SetUniform( "zoom" , glm::mat4(1.) );
    mapWallLayer.SetUniform( "shift" , glm::mat4(1.) );
    mapWallLayer.BindCopyVB(this->verticesTileTriangles,3);
    mapWallLayer.BindCopyIB(this->indicesTileWalls);

    // Grid lines
    GlLayer gridLayer( shaderSolidColor );
    gridLayer.AddInput( "position" , 3 );
    gridLayer.SetUniform( "zoom" , glm::mat4(1.) );
    gridLayer.SetUniform( "shift" , glm::mat4(1.) );
    gridLayer.BindCopyVB(this->verticesGridLines,3);
    gridLayer.BindCopyIB(this->indicesGridLines);

    // Example actor that moves around
    this->tet1.UpdatePos(3,3,this->myMap.h(3,3),-1);
    this->tet1.InitializeVerts(this->myMap);
    GlLayer actorLayer( shaderSolidColor );
    actorLayer.AddInput( "position" , 3 );
    actorLayer.SetUniform( "zoom" , glm::mat4(1.) );
    actorLayer.SetUniform( "shift" , glm::mat4(1.) );
    actorLayer.BindCopyVB(tet1.posVerts,3);

    //int startPos[3] = { tet1.pos[0] , tet1.pos[1] , tet1.pos[2] };
    int eX = 0;
    int eY = 6;
    this->endTraj[0] = eX;
    this->endTraj[1] = eY;
    this->endTraj[2] = myMap.h(eX,eY);
    float vel = VelocityToReach( 3 );

    UpdateTrajectory( this->tet1.pos , this->endTraj , vel );

    // Sample trajectory
    GlLayer trajLayer( shaderSolidColor );
    trajLayer.AddInput( "position" , 3 );
    trajLayer.SetUniform( "zoom" , glm::mat4(1.) );
    trajLayer.SetUniform( "shift" , glm::mat4(1.) );
    trajLayer.BindCopyVB(this->verticesTrajectory,3);
    trajLayer.BindCopyIB(this->indicesTrajectory);


    // Textures
    mapTileLayer.UseThisLayer();
    for ( size_t i = 0 ; i < this->myMap._ydim * this->myMap._xdim + this->myMap._extras.size() ; ++i ) {
      AddSquareTileTextureCoords( verticesTexture );
    }
    mapTileLayer.AddInput( "texCoord" , 2 );
    mapTileLayer.BindCopyVB(this->verticesTexture,2,1);
    mapTileLayer.SetTexture("applyTexture","texCoord","assets/dungeonCrawlSoup/dungeon/floor/lava_0.png",GL_RGBA);

    mapWallLayer.UseThisLayer();
    for ( size_t i = 0 ; i < this->myMap._ydim * this->myMap._xdim + this->myMap._extras.size() ; ++i ) {
      AddSquareTileTextureCoords( verticesTextureWalls );
    }
    mapWallLayer.AddInput( "texCoord" , 2 );
    mapWallLayer.BindCopyVB(this->verticesTextureWalls,2,1);
    mapWallLayer.SetTexture("applyTexture","texCoord","assets/dungeonCrawlSoup/dungeon/wall/hive_0.png",GL_RGBA);

    this->bufferMap = this->myMap;
    std::map<Uint32,bool (*)(const SDL_Event&,BattleMap*)> inputHandler;
    inputHandler[SDL_QUIT] = QuitMap;

    // Wait for the user to quit
    while (!this->quit) {
      //uint32_t frameStart = SDL_GetTicks();

      SDL_Event event;
      while (SDL_PollEvent(&event) > 0) {
        HandleInputEvent( event , this->myMap , inputHandler );
      }
      if ( this->leftButtonDown ) {
        int xNow, yNow;
        uint32_t buttons = SDL_GetMouseState(&xNow,&yNow);
        this->rots[ROTATE_Z].degrees += float(xNow - this->xLast)/2.;
        //this->rots[ROTATE_X].degrees += float(yNow - this->yLast)/10.;
        if ( this->rots[ROTATE_Z].degrees < 0.)   this->rots[ROTATE_Z].degrees += 360.;
        if ( this->rots[ROTATE_Z].degrees > 360.) this->rots[ROTATE_Z].degrees -= 360.;
        if ( buttons & (SDL_BUTTON_LMASK == 0) ) {
          this->leftButtonDown = false;
        }
        this->xLast = xNow;
        this->yLast = yNow;
      }

      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      UpdateViewTransformation();

      DrawTileIdLayer(mapIdLayer);
      DrawMapTiles(mapTileLayer);
      DrawGridLines(gridLayer);

      UpdateTrajectory( this->tet1.pos , this->endTraj , vel );
      DrawTrajectory(trajLayer);

      this->tet1.UpdateLoc(this->myMap);
      std::vector<Translation> ashift;
      ashift.push_back(Translation(1,tet1.location));
      DrawActor(actorLayer,ashift);

      this->verticesMoveOverlay.clear();
      this->indicesMoveOverlay.clear();
      int moveRange = this->tet1.MoveRange();
      int jumpRange = this->tet1.JumpRange();
      jumpRange = 3;
      SetReachable( this->myMap , this->bufferMap , this->tet1.pos[0] , this->tet1.pos[1] , moveRange , jumpRange * HEIGHT_INCREMENT , this->tet1.pos[3] );
      FillReachableVertsInds( this->myMap , this->bufferMap , this->verticesMoveOverlay , this->indicesMoveOverlay );
      DrawMapOverlay(mapTileOverlay);

      DrawMapWalls(mapWallLayer);

      SwapWindows();

      ////glFinish();
      //uint32_t frameEnd = SDL_GetTicks();
      //uint32_t framesUsed = frameEnd - frameStart;

      //if ( framesUsed < TARGET_FRAME_TIME ) {
      //  SDL_Delay(TARGET_FRAME_TIME - framesUsed);
      //}
    }
    return EXIT_SUCCESS;

  }
};

bool QuitMap(const SDL_Event &e , BattleMap* mp) {
  mp->quit = true;
  return true;
}

int main(int argc, char **argv) {
  BattleMap main;
  return main.MainLoop(argc, argv);
}
