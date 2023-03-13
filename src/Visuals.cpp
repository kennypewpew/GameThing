#include "Visuals.h"
#include "Types.h"

#include <string>

// Assumes picutre is right side up -> texture is upside down
Box2Df AtlasInfo::operator()(int x, int y, float xScale, float yScale) {
  Box2Df res;

  float xBlockSize = float(xDim) / float(xMax);
  float yBlockSize = float(yDim) / float(yMax);

  res.x0 = x * xBlockSize;
  res.y1 = y * yBlockSize;
  res.x1 = xScale * (res.x0 + xBlockSize);
  res.y0 = yScale * (res.y1 + yBlockSize);

  return res;
}

Box2Df& AtlasInfo::operator[](std::string name) {
  return nameToCoords[name];
}

