#pragma once

#include <vector>
#include <glm/glm.hpp>

#include <SDL_opengles2.h>
#include <EGL/egl.h>

extern PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
extern PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
extern PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;
extern PFNGLISVERTEXARRAYOESPROC glIsVertexArrayOES;

class Rotation {
 public:
  float degrees;
  glm::vec3 vector;
  Rotation(const float &d, const glm::vec3 &v) : degrees(d) , vector(v) {}
  Rotation() {}
};

enum RotationAxes {
    ROTATE_X
  , ROTATE_Y
  , ROTATE_Z
  , ROTATE_TOTAL
};

class Translation {
 public:
  float distance;
  glm::vec3 vector;
  Translation(const float &d, const glm::vec3 &v) : distance(d) , vector(v) {}
};

struct Pos2D {
  int x;
  int y;
  Pos2D() {}
  Pos2D(int xx, int yy) : x(xx), y(yy) {}
};

struct Pos2Df {
  float x;
  float y;
};

struct Box2D {
  int x0;
  int x1;
  int y0;
  int y1;
};

struct Box2Df {
  float x0;
  float x1;
  float y0;
  float y1;
};


