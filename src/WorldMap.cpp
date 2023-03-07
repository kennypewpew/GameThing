#define WORLDMAP_COMPILE
#include <WorldMap.h>
#include <Types.h>

#include <string>
#include <vector>
#include <queue>

void WorldMapTile::SetType(const std::string &s) {
  type = s;
}
void WorldMapTile::Set(const std::string &s, const std::string& lbl) {
  type = s;
  label = lbl;
}

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



void WorldMap::PopulatePOI( const WorldLocation &poi , WorldMapTileStorage &tiles ) {
  tiles(poi.x , poi.y).Set(poi.type,poi.name);
}

void WorldMap::FillTileBlanks( const std::vector<WorldLocation> &pois , WorldMapTileStorage &tiles ) {
  tiles = GenerateWorldTiles(xDim,yDim);
}

void WorldMap::PopulateTiles() {
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

