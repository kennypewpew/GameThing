#include "BattleActor.h"
#include "Maps.h"

#include <vector>
#include <glm/vec3.hpp>

//float CoordToValue( const float& , const int& );
//float HeightToValue( const float& );

float FACTOR = 1.5;
float CoordToValue(const float &c, const int &max) {
  return FACTOR*float(c)/float(max) - FACTOR/2.;
}

float HFACTOR = 100;
float HeightToValue( const float &h ) {
  return float(h) / HFACTOR;
}


  ActorTetrahedron::ActorTetrahedron() {
    posVerts = std::vector<glm::vec3>(12);
  }

  ActorTetrahedron::ActorTetrahedron(int x , int y , int z) {
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    posVerts = std::vector<glm::vec3>(12);
  }

  void ActorTetrahedron::UpdatePos( int x , int y , int z , int e ) {
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    pos[3] = e;
  }

  int ActorTetrahedron::MoveRange() {
    return this->moveRange;
  }

  int ActorTetrahedron::JumpRange() {
    return this->jumpRange;
  }

  void ActorTetrahedron::UpdateLoc( const Map &mp ) {
    location = glm::vec3( pos[0] * FACTOR / mp._xdim
                        , pos[1] * FACTOR / mp._ydim
                        , float(pos[2]) / HFACTOR
                        );
  }

  void ActorTetrahedron::InitializeVerts( const Map &myMap ) {
     float x1 = CoordToValue( 0   , myMap._xdim );
     float x2 = CoordToValue( 0+1 , myMap._xdim );
     float y1 = CoordToValue( 0   , myMap._ydim );
     float y2 = CoordToValue( 0+1 , myMap._ydim );
     float z1 = 0;
     float x15 = ( x1 + x2 ) / 2.;
     float y15 = ( y1 + y2 ) / 2.;
     posVerts[0] = glm::vec3( x15 , y15 , HeightToValue( z1    ) );
     posVerts[1] = glm::vec3( x1  , y2  , HeightToValue( z1+10 ) );
     posVerts[2] = glm::vec3( x2  , y2  , HeightToValue( z1+10 ) );
     posVerts[3] = glm::vec3( x15 , y15 , HeightToValue( z1    ) );
     posVerts[4] = glm::vec3( x1  , y2  , HeightToValue( z1+10 ) );
     posVerts[5] = glm::vec3( x1  , y1  , HeightToValue( z1+10 ) );
     posVerts[6] = glm::vec3( x15 , y15 , HeightToValue( z1    ) );
     posVerts[7] = glm::vec3( x2  , y2  , HeightToValue( z1+10 ) );
     posVerts[8] = glm::vec3( x1  , y1  , HeightToValue( z1+10 ) );
     posVerts[6] = glm::vec3( x1  , y2  , HeightToValue( z1+10 ) );
     posVerts[7] = glm::vec3( x2  , y2  , HeightToValue( z1+10 ) );
     posVerts[8] = glm::vec3( x1  , y1  , HeightToValue( z1+10 ) );
  }

