#pragma once

#include "Types.h"

#include <map>
#include <string>

class AtlasInfo{
 public:
  int xMax = 0, yMax = 0; // Total pixel size
  int xDim = 0, yDim = 0; // Size of a single texture in pixels

  std::map<std::string,Box2Df> nameToCoords;

  AtlasInfo() {}
  AtlasInfo(int xm, int ym, int xd, int yd) : xMax(xm), yMax(ym), xDim(xd), yDim(yd) {}

  // Assumes picutre is right side up -> texture is upside down
  Box2Df operator()(int x, int y, float xScale = 1., float yScale = 1.);

  Box2Df& operator[](std::string name);
};



