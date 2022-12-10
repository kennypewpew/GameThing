#pragma once

#include "Containers.hpp"
#include "GlLayer.h"

#include <vector>
#include <string>

#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

struct Character {
  char id;
  int16_t x;
  int16_t y;
  int16_t width;
  int16_t height;
  int16_t xoffset;
  int16_t yoffset;
  int16_t xadvance;
};

class CharMap {
 public:
  // Support ascii characters 33 <= c <= 126
  // Manually add space (32)
  std::vector<Character> chars;
  int size;
  int lineHeight;
  int width;
  int height;
  std::string pngFileName;

  CharMap();
  void ReadFontFile(const std::string &dirName, const std::string &flName);
};

float TextLength( const std::string &txt
                , const CharMap &cMap
                , const float &scale = 2.
                );

void WriteText( const std::string &txt
              , std::vector<glm::vec3> &posVec
              , std::vector<GLuint> &posIdVec
              , std::vector<glm::vec2> &posTexVec
              , const CharMap &cMap
              , const float &scale = 2.
              , const float &xLoc = 0.
              , const float &yLoc = 0.
              );

class DisplayText {
 public:
  float xLoc;
  float yLoc;
  float scale;
  std::string text;
  DisplayText( float x , float y , float s , std::string txt );
  DisplayText();
};

class TextWriter {
 private:
  UuidMapper<DisplayText> toWrite;
  bool needsToUpdate;

 public:
  std::vector<glm::vec3> posVerts;
  std::vector<glm::vec2> texVerts;
  std::vector<GLuint> posIds;
  int width;
  int height;

  const CharMap &font;
  glm::vec3 color;
  GlLayer toDraw;

  TextWriter( Shader &shad
            , const CharMap &f
            , const glm::vec3 &c
            , const int &width
            , const int &height
            );

  uint32_t AddText( DisplayText t );

  void RemoveText( uint32_t id );

  void RenderText();
};

class TextBackground {
 public:
  GlLayer toDraw;
  UuidMapper<Box2Df> boxes;
  std::vector<glm::vec3> verts;
  std::vector<GLuint> inds;
  std::vector<glm::vec2> tex;
  bool changed;

  TextBackground(Shader &s);

  uint32_t AddRectangle( const float vx0 , const float vy0
                       , const float vx1 , const float vy1
                       );

  void RemoveRectangle( uint32_t id );

  void ConstructBoxes();

  void Draw();
};


