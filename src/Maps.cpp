#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include <cstdio>

#include <GLES3/gl3.h>
#include <glm/vec3.hpp>

#include "Maps.h"

Map::Map(const int &x, const int &y) {
  _tiles.resize(x*y);
_xdim = x;
_ydim = y;
}
uint8_t Map::h(const int&x, const int&y) const {
  return _tiles[x+y*_xdim].height;
}
uint8_t& Map::h(const int&x, const int&y) {
  return _tiles[x+y*_xdim].height;
}
char Map::t(const int&x, const int&y) const {
  return _tiles[x+y*_xdim].type;
}
char& Map::t(const int&x, const int&y) {
  return _tiles[x+y*_xdim].type;
}

int  Map::x(const int&e) const {
  return _extras[e].x;
}
int& Map::x(const int&e) {
  return _extras[e].x;
}
int  Map::y(const int&e) const {
  return _extras[e].y;
}
int& Map::y(const int&e) {
  return _extras[e].y;
}
uint8_t  Map::h(const int&e) const {
  return _extras[e].height;
}
uint8_t& Map::h(const int&e) {
  return _extras[e].height;
}


bool Map::InBounds( const int &x , const int &y ) {
  return ( x >= 0 && x < _xdim && y >= 0 && y < _ydim );
}

void PrintMap( const Map &mp ) {
  for ( int i = 0 ; i < mp._ydim ; ++i ) {
    for ( int j = 0 ; j < mp._xdim ; ++j ) {
      printf("  %d",mp.h(j,i));
    }
    printf("\n");
  }
  if ( mp._extras.size() ) {
    size_t next = 0;
    for ( int i = 0 ; i < mp._ydim ; ++i ) {
      for ( int j = 0 ; j < mp._xdim ; ++j ) {
        if ( next == mp._extras.size() ) printf("  .");
        else {
          if ( mp._extras[next].x == j && mp._extras[next].y == i ) {
            printf("  %d", mp._extras[next].height);
            ++next;
          }
          else printf("  .");
        }
      }
      printf("\n");
    }
  }
}

Map ReadMapFile( const std::string &mapFile ) {
  std::fstream mapFl;
  mapFl.open(mapFile,std::ios::in);
  // TODO: check
  std::string ln;
  getline(mapFl,ln);
  while ( ln[0] == '#' ) getline(mapFl,ln);
  int space = ln.find(' ');
  int xdim = std::stoi(ln.substr(0,space));
  int ydim = std::stoi(ln.substr(space));
  Map res(xdim,ydim);

  getline(mapFl,ln); // Separator
  getline(mapFl,ln); // Get first line
  while ( ln[0] != '-' ) {
    int dash = ln.find('-');
    std::string label = ln.substr(0,dash);
    std::string name = ln.substr(dash+1);
    getline(mapFl,ln);
  }

  for ( int i = 0 ; i < ydim ; ++i ) {
    getline(mapFl,ln);
    std::stringstream lnStream(ln);
    std::string e;
    for ( int j = 0 ; j < xdim ; ++j ) {
      lnStream >> e;
      res.h(j,i) = 10.*stof(e.substr(0,e.size()-1));
      res.t(j,i) = e.back();
    }
  }
  return res;
}


