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


