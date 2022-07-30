#pragma once
#include <vector>
#include <string>
#include <GLES3/gl3.h>

struct Coord2D {
  int x;
  int y;
};

struct Coord3D {
  int x;
  int y;
  int z;
};

struct MapTile {
  uint8_t height;
  char type;
};

struct ExtraMapTile {
  uint8_t height;
  char type;
  int x;
  int y;
  std::vector<Coord2D> connections;
  std::vector<int> extra_connections;
};

class Map {
 public:
  std::vector<MapTile> _tiles;
  std::vector<ExtraMapTile> _extras;
  int _xdim;
  int _ydim;
  Map() {}
  Map(const int &x, const int &y);
  uint8_t h(const int&x, const int&y) const;
  uint8_t& h(const int&x, const int&y);
  int x(const int&e) const;
  int& x(const int&e);
  int y(const int&e) const;
  int& y(const int&e);
  uint8_t h(const int&e) const;
  uint8_t& h(const int&e);
  char t(const int&x, const int&y) const;
  char& t(const int&x, const int&y);
  bool InBounds( const int &x , const int &y );
};

void PrintMap( const Map &mp );

Map ReadMapFile( const std::string &mapFile );

