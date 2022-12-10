#include <iostream>
#include <vector>
#include <string>

enum Terrain {
    INVALID = 0
  , PLAINS
  , MOUNTAIN
  , FOREST
  , DESERT
  , SWAMP
  , RIVER
  , LAKE
  , OCEAN
  , TERRAIN_TOTAL
};

class WorldMap {
  size_t seed;
  int xDim, yDim;
  std::vector<Terrain> terrain;
  //std::vector<WorldFeature> features;
};

int main(void) {
  // Terrain features
  // Wildlife
  // Population centers
  // Organizations
}
