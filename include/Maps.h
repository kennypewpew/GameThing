#pragma once
#include <vector>
#include <GLES3/gl3.h>


struct MapTile {
  uint8_t height;
  char type;
};


class Map {
 public:
  std::vector<MapTile> _tiles;
  int _xdim;
  int _ydim;
  Map(const int &x, const int &y);
  uint8_t h(const int&x, const int&y) const;
  uint8_t& h(const int&x, const int&y);
  char t(const int&x, const int&y) const;
  char& t(const int&x, const int&y);
};

void PrintMapFile( const Map &mp );

Map ReadMapFile( const std::string &mapFile );

