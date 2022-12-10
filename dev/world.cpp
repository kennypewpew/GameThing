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

#include "Window.h"
#include "GlLayer.h"
#include "Types.h"

const unsigned int DISP_WIDTH = 480;
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
  if      ( key == "default"  ) return TextureCoords(0,0,5,0);
  else if ( key == "forest"   ) return TextureCoords(8,0,5,0);
  else if ( key == "house"    ) return TextureCoords(4,0,7,0);
  else if ( key == "campfire" ) return TextureCoords(7,0,8,0);

  return TextureCoords(0,0,0,0);
}

class WorldLocation {
 public:
  int x;
  int y;
  std::string name;
  std::string type;

  WorldLocation() {}
  WorldLocation( const int &xx
               , const int &yy
               , const std::string &nm
               , const std::string &tp
               ) : x(xx) , y(yy) , name(nm) , type(tp)
               {}
};

class WorldMapTile {
 public:
  std::string type;

  WorldMapTile() {}
  WorldMapTile(const std::string &s) : type(s) {}
  void SetType(const std::string &s) {
    type = s;
  }
};

class WorldMap {
 public:
  int xDim;
  int yDim;
  std::vector<WorldLocation> pointsOfInterest;
  std::vector<WorldMapTile> tiles;

  WorldMap() {
   this->xDim = 0;
   this->yDim = 0;
  }
  WorldMap( const int &x , const int &y ) : xDim(x) , yDim(y) {
    this->tiles.resize(xDim * yDim);
  }
  WorldMap( const int &x
          , const int &y
          , const std::vector<WorldLocation> &poi
          ) : xDim(x) , yDim(y) , pointsOfInterest(poi) {
    this->tiles.resize(xDim * yDim);
  }
  WorldMap( const int &x
          , const int &y
          , const std::vector<WorldLocation> &poi
          , const std::vector<WorldMapTile> &t
          ) : xDim(x) , yDim(y) , pointsOfInterest(poi) , tiles(t) {}

  void PopulatePOI( const WorldLocation &poi , std::vector<WorldMapTile> &tiles ) {
    tiles[poi.x + this->xDim*poi.y].SetType(poi.type);
  }

  void FillTileBlanks( const std::vector<WorldLocation> &pois , std::vector<WorldMapTile> &tiles ) {
    for ( int y = 0 ; y < this->yDim ; ++y ) {
      for ( int x = 0 ; x < this->xDim ; ++x ) {
        if ( !(tiles[x + y*this->xDim].type.size()) ) {
          tiles[x + y*this->xDim].SetType("default");
        }
      }
    }
  }

  void PopulateTiles() {
    if ( !pointsOfInterest.size() ) {
      printf("Can't generate world map without points of interest\n");
      return;
    }
    for ( size_t i = 0 ; i < this->pointsOfInterest.size() ; ++i ) {
      this->PopulatePOI( this->pointsOfInterest[i] , this->tiles );
    }
    this->FillTileBlanks( this->pointsOfInterest , this->tiles );
  }
};

class WorldMapWindow {
 public:
  bool quit;
  bool redraw;
  float zoom;
  float xshift;
  float yshift;
  glm::mat4 view;

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
            //SDL_GetMouseState(&this->xLast,&this->yLast);
            //this->leftButtonDown = true;
            break;
          case SDL_BUTTON_RIGHT:
            {
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

    Shader shadCol( inputColorVS , inputColorFS );
    Shader shadTex( inputTextureVS.c_str() , inputTextureFS.c_str() );

    std::vector<glm::vec3> verCanvas;
    std::vector<GLuint>    indCanvas;
    std::vector<glm::vec3> colCanvas;
    std::vector<glm::vec2> texCanvas;

    //std::vector<std::pair<int,int>> usefulTiles;
    //usefulTiles.push_back(std::pair<int,int>(0,6));
    //usefulTiles.push_back(std::pair<int,int>(1,6));
    //usefulTiles.push_back(std::pair<int,int>(2,6));
    //usefulTiles.push_back(std::pair<int,int>(3,6));
    //usefulTiles.push_back(std::pair<int,int>(4,6));
    //usefulTiles.push_back(std::pair<int,int>(5,6));
    //usefulTiles.push_back(std::pair<int,int>(6,6));
    //usefulTiles.push_back(std::pair<int,int>(7,6));
    //usefulTiles.push_back(std::pair<int,int>(8,6));
    //usefulTiles.push_back(std::pair<int,int>(9,6));

    // Input texture is 10x28
    float texModX = 1./10.;
    float texModY = 1./28.;
    float xSz = 1.;
    float ySz = 1.;

    int xDim = 21;
    int yDim = 41;
    int cnt = 0;

    std::vector<WorldLocation> locations;
    locations.push_back(WorldLocation(3 ,3,"name1","forest"));
    locations.push_back(WorldLocation(14,26,"name2","house"));
    locations.push_back(WorldLocation(5,39,"name3","campfire"));
    WorldMap worldMap(xDim,yDim,locations);
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
        TextureCoords t = GetTexCoordsFromName( tileTranslator , worldMap.tiles[x + y*xDim].type );
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
