#pragma once

#include "Maps.h"

#include <vector>
#include <glm/vec3.hpp>

float CoordToValue(const float &c, const int &max);
float HeightToValue( const float &h );

class ActorTetrahedron {
 public:
  std::vector<glm::vec3> posVerts;
  int pos[4];
  glm::vec3 location;
  int moveRange = 2;
  int jumpRange = 2;

  ActorTetrahedron();

  ActorTetrahedron(int x , int y , int z);

  void UpdatePos( int x , int y , int z , int e );

  void UpdateLoc( const Map &mp );

  int MoveRange();
  int JumpRange();

  void InitializeVerts( const Map &myMap );
};


