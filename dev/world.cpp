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

enum TerrainTypes {
    TERRAIN_INVALID
  , GRASS
  , SAND
  , MOUNTAIN
  , FOREST
  , BOTTLE
  , SCROLL
  //, OCEAN
  //, LAKE
  //, RIVER
  , TERRAIN_COUNT
};

const char *TerrainEnumMap[] = {
    ""
  , "grass"
  , "sand"
  , "mountain"
  , "forest"
  , "bottle"
  , "scroll"
  //, "water"
  //, "water"
  //, "water"
  , ""
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
  std::string type,label;

  WorldMapTile() {}
  WorldMapTile(const std::string &s) : type(s) {}
  WorldMapTile(const std::string &s, const std::string& lbl) : type(s), label(lbl) {}
  void SetType(const std::string &s) {
    type = s;
  }
  void Set(const std::string &s, const std::string& lbl) {
    type = s;
    label = lbl;
  }
};

class WorldMapTileStorage {
 public:
  std::vector<WorldMapTile> tiles;
  int xDim, yDim;
  WorldMapTileStorage() : xDim(0), yDim(0) {}
  WorldMapTileStorage(int x, int y) : xDim(x), yDim(y), tiles(std::vector<WorldMapTile>(x*y)) {}

  WorldMapTile& operator()(int x, int y) { return tiles[x + y*xDim]; }
};

void FillBorders(WorldMapTileStorage &tiles, std::string terrain = "water") {
  int xDim = tiles.xDim;
  int yDim = tiles.yDim;
  for ( int x = 0 ; x < xDim ; ++x ) {
    tiles(x , 0     ).SetType(terrain);
    tiles(x , yDim-1).SetType(terrain);
  }
  for ( int y = 0 ; y < yDim ; ++y ) {
    tiles(0      , y).SetType(terrain);
    tiles(xDim-1 , y).SetType(terrain);
  }
}

// Probability of expanding from central point x:
// 0.05 0.2 0.05 
// 0.2   x   0.2   
// 0.05 0.2 0.05  
//
// Probability of removed points is distributed equally to remaining points
//   weighted by their original probabilities
const float expandProbabilities[3][3] = {
    { 0.05 , 0.2 , 0.05 }
  , { 0.2  ,  0  , 0.2  }
  , { 0.05 , 0.2 , 0.05 }
};

Pos2D ExpandTile(WorldMapTileStorage &tiles, Pos2D p) {
  bool empty[3][3] = {
      { true , true , true }
    , { true , true , true }
    , { true , true , true }
  };

  int x = p.x;
  int y = p.y;

  float totalProb = 0.;
  int setCount = 0;
  for ( int i = 0 ; i < 3 ; ++i ) {
    for ( int j = 0 ; j < 3 ; ++j ) {
      if (  ((x+i-1) < 0) || ((x+i-1)==tiles.xDim)
         || ((y+j-1) < 0) || ((y+j-1)==tiles.yDim)
         || tiles(x+i-1,y+j-1).type.size() ) {
        ++setCount;
        empty[i][j] = false;
      }
      else {
        totalProb += expandProbabilities[i][j];
      }
    }
  }

  // Nowhere to go
  if ( setCount == 9 ) return { -1, -1 };

  int r = rand() % 100;
  float cumProb = 0.;
  for ( int i = 0 ; i < 3 ; ++i ) {
    for ( int j = 0 ; j < 3 ; ++j ) {
      float probHere = empty[i][j] * expandProbabilities[i][j] / totalProb;
      cumProb += probHere;
      if ( int(100.*cumProb) > r ) return { x + i - 1 , y + j - 1 };
    }
  }

  return { x + 1 , y + 1 };
}

int NQueuesNotEmpty(const std::vector<std::queue<Pos2D>> &qs) {
  int res = 0;
  for ( size_t i = 0 ; i < qs.size() ; ++i ) {
    if ( qs[i].size() ) ++res;
  }
  return res;
}

void SeedAndExpandTerrains(WorldMapTileStorage &tiles) {
  std::vector<std::queue<Pos2D>> expansionQueue(TERRAIN_COUNT);
  for ( int n = 0 ; n < 2 ; ++n )
    for ( int i = 1 ; i < TERRAIN_COUNT ; ++i ) {
      int x = 0;
      int y = 0;
      do {
        x = rand() % tiles.xDim;
        y = rand() % tiles.yDim;
      } while ( tiles(x,y).type.size() );
  
      tiles(x,y).SetType(TerrainEnumMap[i]);
      expansionQueue[i].push( { x,y } );
      printf("Putting %s at %d,%d\n", TerrainEnumMap[i], x,y);
    }

  while ( NQueuesNotEmpty(expansionQueue) ) {
    for ( int i = 1 ; i < TERRAIN_COUNT ; ++i ) {
      if ( expansionQueue[i].size() == 0 ) continue;
      Pos2D p = expansionQueue[i].front();
      expansionQueue[i].pop();
  
      Pos2D pNext = ExpandTile(tiles,p);
      if ( pNext.x != -1 ) {
        tiles(pNext.x,pNext.y).SetType(TerrainEnumMap[i]);
        expansionQueue[i].push(pNext);
        expansionQueue[i].push(p);
      }
    }
  }
}

WorldMapTileStorage GenerateWorldTiles( int xDim
                                      , int yDim
                                      , int seed = 0
                                      ) {
  srand(seed);

  WorldMapTileStorage tiles(xDim, yDim);

  //FillBorders(tiles);
  SeedAndExpandTerrains(tiles);

  return tiles;
}



class WorldMap {
 public:
  int xDim;
  int yDim;
  std::vector<WorldLocation> pointsOfInterest;
  WorldMapTileStorage tiles;

  WorldMap() {
   this->xDim = 0;
   this->yDim = 0;
  }
  WorldMap( const int &x , const int &y ) : xDim(x) , yDim(y) , tiles(x,y) {}
  WorldMap( const int &x
          , const int &y
          , const std::vector<WorldLocation> &poi
          ) : xDim(x) , yDim(y) , pointsOfInterest(poi) , tiles(x,y) {}
  WorldMap( const int &x
          , const int &y
          , const std::vector<WorldLocation> &poi
          , const WorldMapTileStorage &t
          ) : xDim(x) , yDim(y) , pointsOfInterest(poi) , tiles(t) {}

  void PopulatePOI( const WorldLocation &poi , WorldMapTileStorage &tiles ) {
    tiles(poi.x , poi.y).Set(poi.type,poi.name);
  }

  void FillTileBlanks( const std::vector<WorldLocation> &pois , WorldMapTileStorage &tiles ) {
    tiles = GenerateWorldTiles(xDim,yDim);
  }

  void PopulateTiles() {
    std::vector<WorldLocation> locations;
    for ( int i = 0 ; i < 10 ; ++i ) {
      int x = rand() % xDim;
      int y = rand() % yDim;
      std::string label = std::string("name") + std::to_string(i);
      locations.push_back(WorldLocation(x,y,label,"house"));
    }
    pointsOfInterest = locations;
    //if ( !pointsOfInterest.size() ) {
    //  printf("Can't generate world map without points of interest\n");
    //  return;
    //}
    this->FillTileBlanks( this->pointsOfInterest , this->tiles );
    for ( size_t i = 0 ; i < this->pointsOfInterest.size() ; ++i ) {
      this->PopulatePOI( this->pointsOfInterest[i] , this->tiles );
    }
  }
};

class GlobalActor {
 public:
  size_t uid;
  std::map<size_t,float> relations;

  GlobalActor() {}
  GlobalActor(size_t id) : uid(id) {}
  ~GlobalActor() {}

  void DecayRelations(float rate = 0.95) {
    for ( auto &r : relations ) {
      r.second *= rate;
    }
  }
  void ModifyRelation(size_t with, float change) {
    if ( !relations.count(with) ) relations[with] = 0.;
    relations[with] += change;
  }
  virtual void TakeAction() = 0;
};

enum GlobalActorType {
    GLOBAL_ACTOR_INVALID
  , GLOBAL_ACTOR_FIXED
  , GLOBAL_ACTOR_MOBILE
  , GLOBAL_ACTOR_CONTAINER
  , N_GLOBAL_ACTOR
};

class GlobalActorTracker {
 public:
  UuidMapper<std::shared_ptr<GlobalActor>,size_t> allGlobalActors; 
  GlobalActorType type;

  GlobalActorTracker() {}
  ~GlobalActorTracker() {}

  template<typename T, class... Args>
  std::shared_ptr<T> CreateActor(Args&&... args) {
    std::shared_ptr<T> ret = std::make_shared<T>(args...);
    size_t id = allGlobalActors.Insert(ret);
    ret->uid = id;
    return ret;
  }

  void RemoveActor(size_t id) {
    allGlobalActors.Delete(id);
  }

  GlobalActorType Type() {
    return type;
  }

  void Act() {
    for ( auto &a : allGlobalActors.map ) a.second->TakeAction();
  }

  template<typename T>
  T* Ptr(size_t id) {
    return (T*)(allGlobalActors.map[id].get());
  }
};

GlobalActorTracker actors;

class City : public GlobalActor {
 public:
  std::string name;
  size_t population;
  float wealth;
  float security;
  float happiness;
  // disposition

  City() : wealth(10.) , security(3.) , happiness(50.) {}

  void UpdateHappiness() {
    happiness += 0.1 * (wealth * security - happiness);
  }

  void TakeAction() {
    UpdateHappiness();
    printf("City action:\n");
    printf("\t  w   |   s   |   h\n\t%1.3f | %1.3f | %1.3f\n", wealth, security, happiness);
  }
};

class Task {
 public:
  Pos2D loc;
  TaskType type;
  float difficulty;
  size_t targetUid;
  int duration;

  Task() : type(TASK_NONE) {}
  Task( TaskType t , Pos2D l , float d , int dur = 0 )
      : type(t), loc(l), difficulty(d), targetUid(0), duration(dur) {}
  Task( TaskType t , size_t i , float d , int dur = 0 )
      : type(t), targetUid(i), difficulty(d), loc({-1,-1}), duration(dur) {}
};

class Quest {
 public:
  std::queue<Task> steps;

  Task& CurrentStep() {
    if ( !steps.size() ) steps.push(Task());
    return steps.front();
  }
  void StepComplete() {
    steps.pop();
  }
  void AddStep(Task step) {
    steps.push(step);
  }
};

class Party : public GlobalActor {
 public:
  std::vector<std::shared_ptr<Adventurer>> members;
  Inventory inventory;
  Quest activeQuest;
  Quest longerTermQuest;
  size_t parentActorId;
  Pos2Df location;
  bool hidden;

  void TakeAction() {
    printf("Party action:\n");
    // Check for nearby parties and act accordingly
    //   If no action needed, proceed with activeQuest

    bool reacted = ReactToWorld();
    if ( !reacted ) ContinueTask();

  }

  bool ReactToWorld() {
    return false;
  }

  bool MoveTowards(Pos2D p ) {
    Pos2Df pp = { float(p.x) , float(p.y) };
    return MoveTowards(pp);
  }

  bool MoveTowards(Pos2Df p) {
    float xDiff = p.x - location.x;
    float yDiff = p.y - location.y;

    bool xReached = abs(xDiff) < 0.01;
    bool yReached = abs(yDiff) < 0.01;

    // Avoid division by zero
    float xProp = xReached ? 0 : xDiff / ( abs(xDiff) + abs(yDiff) );
    float yProp = yReached ? 0 : yDiff / ( abs(xDiff) + abs(yDiff) );

    location.x += Speed() * xProp;
    location.y += Speed() * yProp;

    float xDiffNew = p.x - location.x;
    float yDiffNew = p.y - location.y;

    // Avoid overshoot oscillations
    if ( xDiffNew * xDiff < 0 ) {
      location.x = p.x;
      xReached = true;
    }
    if ( yDiffNew * yDiff < 0 ) {
      location.y = p.y;
      yReached = true;
    }

    return xReached & yReached;
  }

  void ContinueTask() {
    Task &task = activeQuest.CurrentStep();
    TaskType t = task.type;
    if ( TASK_NONE == t ) {
      // Decide a new task for yourself
      //   For now, think and then go somewhere random
      activeQuest.StepComplete();
      int d = rand() % 10;
      activeQuest.AddStep( Task( WAIT , {0,0} , 1. , d ) );
      int x = rand() % MAPXDIM;
      int y = rand() % MAPYDIM;
      activeQuest.AddStep( Task( GOHERE , {x,y} , 1. ) );
    }
    else if ( GOHERE    == t ) {
      bool reached = MoveTowards(task.loc);
      if ( reached ) activeQuest.StepComplete();
    }
    else if ( INTERCEPT  == t ) {
    }
    else if ( FETCH == t ) {
    }
    else if ( DELIVER   == t ) { }
    else if ( ATTACK    == t ) { }
    else if ( DEFEND    == t ) {
      actors.Ptr<City>(1)->security += PowerLevel();
    }
    else if ( DISCOVER  == t ) { }
    else if ( SCOUT     == t ) { }
    else if ( BUILD     == t ) { }
    else if ( WAIT      == t ) {
      --task.duration;
      if ( task.duration <= 0 ) activeQuest.StepComplete();
    }
    else {} // unsupported task
  }

  void Die() {
    //if ( parentActorId )
    //actors[parentActorId].ModifyAccordingly();
  }

  float Speed() { return 1.; }
  float PowerLevel() { return 1.; }
  float VisionRange() { return 10.; }
};

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
