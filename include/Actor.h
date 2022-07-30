#pragma once

#include "Maps.h"

#include <vector>
#include <glm/vec3.hpp>

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

  void UpdateLoc( const Map &mp , const float &FACTOR , const float &HFACTOR );

  int MoveRange();
  int JumpRange();

  void InitializeVerts( const Map &myMap );
};


