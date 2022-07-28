#include "MapGen.h"
#include "Maps.h"

#include <cstdlib>
#include <string>
#include <vector>
#include <cmath>

void SeedFeatureFLAT( Map &m ) {
  int x = rand() % m._xdim;
  int y = rand() % m._ydim;
  m.h(x,y) = 0;
}

void SeedFeatureHILL( Map &m ) {
  int w = m._xdim;
  int l = m._ydim;
  int x = rand() % w;
  int y = rand() % l;
  int hmax = w > l ? w : l;
  hmax *= 5;
  //m.h(x,y) = hmax;
  for ( int i = 0 ; i < w ; ++i ) {
    m.h(i,y) = hmax - std::abs(x-i);
  }
  for ( int i = 0 ; i < l ; ++i ) {
    m.h(x,i) = hmax - std::abs(y-i);
  }
}

void SeedFeatureMOUNTAIN( Map &m ) {
  int w = m._xdim;
  int l = m._ydim;
  int x = rand() % w;
  int y = rand() % l;
  int hmax = w > l ? w : l;
  hmax *= 4;
  //m.h(x,y) = hmax;
  for ( int i = 0 ; i < w ; ++i ) {
    m.h(i,y) = hmax - 4*std::abs(x-i);
  }
  for ( int i = 0 ; i < l ; ++i ) {
    m.h(x,i) = hmax - 4*std::abs(y-i);
  }

  ExtraMapTile e;
  e.x = rand() % w;
  e.y = rand() % l;
  e.height = 0;
  e.extra_connections.push_back(1);
  m._extras.push_back(e);
  e.y += 1;
  e.extra_connections.push_back(0);
  e.extra_connections[0] = 2;
  m._extras.push_back(e);
  e.x += 1;
  e.extra_connections.clear();
  e.extra_connections.push_back(1);
  e.connections.push_back( (Coord2D){e.x+1,e.y} );
  m._extras.push_back(e);
}

// Seed the initial features to ensure the features happen
void SeedFeatures( Map &m , const std::vector<VerticalityFeatures> &ft ) {
  for ( size_t i = 0 ; i < ft.size() ; ++i ) {
    if ( ft[i] == FLAT ) SeedFeatureFLAT(m);
    if ( ft[i] == HILL ) SeedFeatureHILL(m);
    if ( ft[i] == MOUNTAIN ) SeedFeatureMOUNTAIN(m);
  }
}

// Propagate opposite the direction of the loop, otherwise we just fill in the topleftmost value
bool PropagateTile(Map &m,int x,int y) {
  if ( (y != m._ydim-1) && (m.h(x,y+1)) != uint8_t(-1) ) { m.h(x,y) = m.h(x,y+1); return true;  }
  if ( (x != m._xdim-1) && (m.h(x+1,y)) != uint8_t(-1) ) { m.h(x,y) = m.h(x+1,y); return true;  }
  if ( (y != 0)         && (m.h(x,y-1)) != uint8_t(-1) ) { m.h(x,y) = m.h(x,y-1); return true;  }
  if ( (x != 0)         && (m.h(x-1,y)) != uint8_t(-1) ) { m.h(x,y) = m.h(x-1,y); return true;  }
  return false;
}

// Propagate the map outside of the features
void PropagateTiles( Map &m ) {
  bool changed = true;
  while ( changed ) {
    changed = false;
    for ( int x = 0 ; x < m._xdim ; ++x ) {
      for ( int y = 0 ; y < m._ydim ; ++y ) {
        if ( m.h(x,y) == uint8_t(-1) ) changed = PropagateTile(m,x,y);
      }
    }
  }
}

// Make sure all(?) tiles are reachable
void EnsurePassage( Map &m ) {
}

Map GenerateMap( const int &w , const int &l , const std::vector<VerticalityFeatures> &ft ) {
  Map m(w,l);
  for ( int i = 0 ; i < w*l ; ++i ) {
    m._tiles[i].height = -1;
  }

  SeedFeatures(m,ft);
  PropagateTiles(m);
  EnsurePassage(m);

  return m;
}

