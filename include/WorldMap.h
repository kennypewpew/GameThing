#pragma once

#include <string>
#include <vector>

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

#ifdef WORLDMAP_COMPILE
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
#endif

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
  void SetType(const std::string &s);
  void Set(const std::string &s, const std::string& lbl);

};

class WorldMapTileStorage {
 public:
  std::vector<WorldMapTile> tiles;
  int xDim, yDim;
  WorldMapTileStorage() : xDim(0), yDim(0) {}
  WorldMapTileStorage(int x, int y) : xDim(x), yDim(y), tiles(std::vector<WorldMapTile>(x*y)) {}

  inline WorldMapTile& operator()(int x, int y) { return tiles[x + y*xDim]; }
};


class WorldMap {
 public:
  int xDim;
  int yDim;
  std::vector<WorldLocation> pointsOfInterest;
  WorldMapTileStorage tiles;

  WorldMap() : xDim(0), yDim(0) {}
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

  void PopulatePOI( const WorldLocation &poi , WorldMapTileStorage &tiles );

  void FillTileBlanks( const std::vector<WorldLocation> &pois , WorldMapTileStorage &tiles );

  void PopulateTiles();
};


