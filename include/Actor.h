#pragma once

#include "Maps.h"

#include <vector>
#include <glm/vec3.hpp>

class ActorTetrahedron {
 public:
  std::vector<glm::vec3> posVerts;
  int pos[4];
  glm::vec3 location;

  ActorTetrahedron();

  ActorTetrahedron(int x , int y , int z);

  void UpdatePos( int x , int y , int z , int e );

  void UpdateLoc( const Map &mp , const float &FACTOR , const float &HFACTOR );

  void InitializeVerts( const Map &myMap );
};


