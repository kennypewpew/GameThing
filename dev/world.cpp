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
#include <queue>
#include <cstdlib>
#include <memory>

#include "Window.h"
#include "GlLayer.h"
#include "Types.h"
#include "Stats.h"
#include "Containers.hpp"
#include "WorldMap.h"
#include "GlobalActor.h"

const int MAPXDIM = 30;
const int MAPYDIM = 30;

const unsigned int DISP_WIDTH = 960;
const unsigned int DISP_HEIGHT = 960;

const unsigned int TARGET_FPS = 1;
const unsigned int TARGET_FRAME_TIME = 1000 / TARGET_FPS;

class TextureCoords {
 public:
  float x1,x2,y1,y2;
  TextureCoords() {}
  TextureCoords( const float &xx1
               , const float &xx2
               , const float &yy1
               , const float &yy2
               ) : x1(xx1) , x2(xx2) , y1(yy1) , y2(yy2)
               {}
};

TextureCoords GetTexCoordsFromName( const std::map<std::string,std::pair<int,int>> &translate , const std::string &key ) {
  if      ( key == "grass"    ) return TextureCoords(0,0,5,0);
  else if ( key == "water"    ) return TextureCoords(0,0,7,0);
  else if ( key == "sand"     ) return TextureCoords(4,0,5,0);
  else if ( key == "forest"   ) return TextureCoords(8,0,5,0);
  else if ( key == "mountain" ) return TextureCoords(6,0,6,0);
  else if ( key == "house"    ) return TextureCoords(4,0,7,0);
  else if ( key == "campfire" ) return TextureCoords(7,0,8,0);
  else if ( key == "bottle"   ) return TextureCoords(8,0,9,0);
  else if ( key == "scroll"   ) return TextureCoords(0,0,11,0);
  else if ( key == "party"    ) return TextureCoords(0,0,19,0);
  else if ( key == "monster"  ) return TextureCoords(3,0,19,0);

  return TextureCoords(0,0,0,0);
}

void updateMobileVertices(glm::vec3 *verts, Pos2Df &pos, int xDim, int yDim) {
    float x = pos.x;
    float y = pos.y;
    float x1 = float(x+0)/xDim * 2. - 1;
    float y1 = float(y+0)/yDim * 2. - 1;
    float x2 = float(x+1)/xDim * 2. - 1;
    float y2 = float(y+1)/yDim * 2. - 1;
    verts[0] = glm::vec3(x1,y1,0.0);
    verts[1] = glm::vec3(x1,y2,0.0);
    verts[2] = glm::vec3(x2,y1,0.0);
    verts[3] = glm::vec3(x2,y2,0.0);
}

class WorldMapWindow {
 public:
  bool quit;
  bool redraw;
  float zoom;
  float xshift;
  float yshift;
  glm::mat4 view;

  int xDim, yDim;
  WorldMap worldMap;

  WorldMapWindow() {
    InitializeSDL("World Map",DISP_WIDTH,DISP_HEIGHT);
    this->zoom = 1.;
    this->xshift = 0.;
    this->yshift = 0.;
    this->quit = false;
    this->view = glm::mat4(1.f);
  }

  void HandleInputEvent( const SDL_Event &e ) {
      if (e.type == SDL_QUIT) {
        this->quit = true;
      }
      else if ( e.type == SDL_KEYDOWN ) {
        switch (e.key.keysym.sym)
        {
          case SDLK_PERIOD:
            actors.Act();
            break;
          case SDLK_SPACE:
            actors.Act();
            break;
          case SDLK_a:
            //SpinMap( -15. );
            break;
          case SDLK_d:
            //SpinMap( +15. );
            break;
          case SDLK_s:
            //TiltMap( -5. );
            break;
          case SDLK_w:
            //TiltMap( +5. );
            break;
          case SDLK_UP:
            //ShiftMap( +0.05f , 0 );
            break;
          case SDLK_DOWN:
            //ShiftMap( -0.05f , 0 );
            break;
          case SDLK_LEFT:
            //ShiftMap( -0.05f , 90 );
            break;
          case SDLK_RIGHT:
            //ShiftMap( +0.05f , 90 );
            break;
          case SDLK_y:
            //Zoom(+0.05f);
            break;
          case SDLK_t:
            //Zoom(-0.05f);
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
            {
            int xLast, yLast;
            SDL_GetMouseState(&xLast,&yLast);
            //this->leftButtonDown = true;
            int xCvt = DISP_WIDTH / xDim;
            int yCvt = DISP_HEIGHT / yDim;
            //printf("%d , %d\n", xLast, yLast);
            int x = xLast / xCvt;
            int y = yDim - 1 - yLast / yCvt;
            //printf("%d , %d: %s  %s\n", x, y, worldMap.tiles(x,y).type.c_str(), worldMap.tiles(x,y).label.c_str());
    Quest q;
    q.AddStep( Task( GOHERE , {x,y} , 1. ) );
    actors.Ptr<Party>(2)->activeQuest = q;
            actors.Act();
            break;
            }
          case SDL_BUTTON_RIGHT:
            {
            int xLast, yLast;
            SDL_GetMouseState(&xLast,&yLast);
            int xCvt = DISP_WIDTH / xDim;
            int yCvt = DISP_HEIGHT / yDim;
            int x = xLast / xCvt;
            int y = yDim - 1 - yLast / yCvt;
            //printf("%d , %d: %s  %s\n", x, y, worldMap.tiles(x,y).type.c_str(), worldMap.tiles(x,y).label.c_str());
    Quest q;
    q.AddStep( Task( GOHERE , {x,y} , 1. ) );
    actors.Ptr<Party>(3)->activeQuest = q;
            actors.Act();
              //GetTileCoords( coordinateBuffer, map , tet1.pos );
              //GetTileCoords( coordinateBuffer, map , this->endTraj );
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
            //this->leftButtonDown = false;
            break;
            case SDL_BUTTON_RIGHT:
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
      else if ( e.type == SDL_MOUSEWHEEL ) {
        if (e.wheel.y > 0 || e.mgesture.dDist > 0 ) {
          //Zoom(+0.05f);
        }
        else if (e.wheel.y < 0 ) {
          //Zoom(-0.05f);
        }
      }
      else if ( e.type == SDL_MULTIGESTURE ) {
        if ( e.mgesture.dDist > 0 ) {
          //Zoom( e.mgesture.dDist/500. );
        }
        else if ( e.mgesture.dDist < 0 ) {
          //Zoom( e.mgesture.dDist/500. );
        }
      }
  }


  int MainLoop() {
    std::string inputColorVS = shaderFileToString( "shader/inputColor.vs" );
    std::string inputColorFS = shaderFileToString( "shader/inputColor.fs" );
    std::string inputTextureVS = shaderFileToString( "shader/inputTexture.vs" );
    std::string inputTextureFS = shaderFileToString( "shader/inputTexture.fs" );

    Shader shadCol( inputColorVS.c_str() , inputColorFS.c_str() );
    Shader shadTex( inputTextureVS.c_str() , inputTextureFS.c_str() );

    std::vector<glm::vec3> verCanvas;
    std::vector<GLuint>    indCanvas;
    std::vector<glm::vec2> texCanvas;

    // Input texture is 10x28
    float texModX = 1./10.;
    float texModY = 1./28.;
    float xSz = 1.;
    float ySz = 1.;

    this->xDim = MAPXDIM;
    this->yDim = MAPYDIM;
    int cnt = 0;

    worldMap = WorldMap(xDim,yDim);
    worldMap.PopulateTiles();
    std::map<std::string,std::pair<int,int>> tileTranslator;

    for ( int y = 0 ; y < yDim ; ++y ) {
      for ( int x = 0 ; x < xDim ; ++x ) {
        float x1 = float(x+0)/xDim * 2. - 1;
        float y1 = float(y+0)/yDim * 2. - 1;
        float x2 = float(x+1)/xDim * 2. - 1;
        float y2 = float(y+1)/yDim * 2. - 1;

        verCanvas.push_back(glm::vec3(x1,y1,0.0));
        verCanvas.push_back(glm::vec3(x1,y2,0.0));
        verCanvas.push_back(glm::vec3(x2,y1,0.0));
        verCanvas.push_back(glm::vec3(x2,y2,0.0));

        indCanvas.push_back(4*cnt + 0);
        indCanvas.push_back(4*cnt + 1);
        indCanvas.push_back(4*cnt + 2);
        indCanvas.push_back(4*cnt + 1);
        indCanvas.push_back(4*cnt + 2);
        indCanvas.push_back(4*cnt + 3);

        //float xLoc = float(x % 10);
        //float yLoc = 5.;
        TextureCoords t = GetTexCoordsFromName( tileTranslator , worldMap.tiles(x , y).type );
        float xLoc = t.x1;
        float yLoc = t.y1;

        texCanvas.push_back(glm::vec2((0.0+xLoc)*texModX,(ySz+yLoc)*texModY));
        texCanvas.push_back(glm::vec2((0.0+xLoc)*texModX,(0.0+yLoc)*texModY));
        texCanvas.push_back(glm::vec2((xSz+xLoc)*texModX,(ySz+yLoc)*texModY));
        texCanvas.push_back(glm::vec2((xSz+xLoc)*texModX,(0.0+yLoc)*texModY));
        ++cnt;
      }
    }

    GlLayer mapBg( shadTex );
    mapBg.AddInput( "position" , 3 );
    mapBg.SetUniform( "zoom" , glm::mat4(1.) );
    mapBg.SetUniform( "shift" , glm::mat4(1.) );
    mapBg.BindCopyVB(verCanvas,3,0);
    mapBg.BindCopyIB(indCanvas);

    // Textures
    mapBg.UseThisLayer();
    mapBg.AddInput( "texCoord" , 2 );
    mapBg.BindCopyVB(texCanvas,2,1);
    //mapBg.SetTexture("applyTexture","texCoord","assets/tiles/clover.jpg",GL_RGB);
    mapBg.SetTexture("applyTexture","texCoord","assets/tileset.png",GL_RGBA);

    auto a1 = actors.CreateActor<City>();
    auto a2 = actors.CreateActor<Party>();
    printf("%lu | %lu\n", a1->uid, a2->uid);
    a1->ModifyRelation(3,100.);
    a1->ModifyRelation(2,-3.);
    a1->DecayRelations();
    for ( auto r : a1->relations ) {
      printf("\t%lu : %f\n", r.first , r.second);
    }
    Quest q;
    q.AddStep( Task( GOHERE , {1,10} , 1. ) );
    q.AddStep( Task( GOHERE , {10,1} , 1. ) );
    a2->activeQuest = q;

    auto a3 = actors.CreateActor<Party>();
    Quest q2;
    q2.AddStep( Task( GOHERE , {10,1} , 1. ) );
    q2.AddStep( Task( GOHERE , {1,10} , 1. ) );
    a3->activeQuest = q2;

    std::vector<glm::vec3> verActors(8);
    std::vector<GLuint>    indActors;
    std::vector<glm::vec2> texActors;

    updateMobileVertices( &verActors[0], a2->location, xDim, yDim );
    indActors.push_back(0);
    indActors.push_back(1);
    indActors.push_back(2);
    indActors.push_back(1);
    indActors.push_back(2);
    indActors.push_back(3);
    TextureCoords t = GetTexCoordsFromName( tileTranslator , "party" );
    float xLoc = t.x1;
    float yLoc = t.y1;
    texActors.push_back(glm::vec2((0.0+xLoc)*texModX,(ySz+yLoc)*texModY));
    texActors.push_back(glm::vec2((0.0+xLoc)*texModX,(0.0+yLoc)*texModY));
    texActors.push_back(glm::vec2((xSz+xLoc)*texModX,(ySz+yLoc)*texModY));
    texActors.push_back(glm::vec2((xSz+xLoc)*texModX,(0.0+yLoc)*texModY));

    updateMobileVertices( &verActors[4], a3->location, xDim, yDim );
    indActors.push_back(4);
    indActors.push_back(5);
    indActors.push_back(6);
    indActors.push_back(5);
    indActors.push_back(6);
    indActors.push_back(7);
    t = GetTexCoordsFromName( tileTranslator , "monster" );
    xLoc = t.x1;
    yLoc = t.y1;
    texActors.push_back(glm::vec2((0.0+xLoc)*texModX,(ySz+yLoc)*texModY));
    texActors.push_back(glm::vec2((0.0+xLoc)*texModX,(0.0+yLoc)*texModY));
    texActors.push_back(glm::vec2((xSz+xLoc)*texModX,(ySz+yLoc)*texModY));
    texActors.push_back(glm::vec2((xSz+xLoc)*texModX,(0.0+yLoc)*texModY));
 
    GlLayer mobileParties( shadTex );
    mobileParties.AddInput( "position" , 3 );
    mobileParties.SetUniform( "zoom" , glm::mat4(1.) );
    mobileParties.SetUniform( "shift" , glm::mat4(1.) );
    mobileParties.BindCopyVB(verActors,3,0);
    mobileParties.BindCopyIB(indActors);
    mobileParties.UseThisLayer();
    mobileParties.AddInput( "texCoord" , 2 );
    mobileParties.BindCopyVB(texActors,2,1);
    mobileParties.SetTexture("applyTexture","texCoord","assets/tileset.png",GL_RGBA);

    while (!this->quit) {
      //uint32_t frameStart = SDL_GetTicks();

      SDL_Event event;
      while (SDL_PollEvent(&event) > 0) {
        HandleInputEvent( event );
      }

      glClear(GL_COLOR_BUFFER_BIT);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      mapBg.UseThisLayer();
      mapBg.SetUniform( "view" , this->view );
      mapBg.BindCopyVB( verCanvas , 3 );
      mapBg.BindCopyIB( indCanvas);
      glDrawElements(GL_TRIANGLES,indCanvas.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

      updateMobileVertices( &verActors[0], a2->location, xDim, yDim );
      updateMobileVertices( &verActors[4], a3->location, xDim, yDim );
      mobileParties.UseThisLayer();
      mobileParties.SetUniform( "view" , this->view );
      mobileParties.BindCopyVB( verActors , 3 );
      mobileParties.BindCopyIB( indActors);
      glDrawElements(GL_TRIANGLES,indActors.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

      //UpdateViewTransformation();
      //DrawMapOverlay(mapTileOverlay);
      SwapWindows();

    }
    return EXIT_SUCCESS;
  }

};

int main(int argc, char **argv) {
  WorldMapWindow main;
  main.MainLoop();
}
