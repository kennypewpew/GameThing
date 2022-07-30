#include "Physics.h"

#include <glm/glm.hpp>

float CoordsToDistance( const int &x1 , const int &y1 , const int &x2 , const int &y2 ) {
  int xdiff = x1 - x2;
  int ydiff = y1 - y2;
  return glm::sqrt( xdiff*xdiff + ydiff*ydiff );
}

// v = (r/cos()) / sqrt( 2 * (tan() * r + y_0) )
float VelocityToReach( const float &r ) {
  return sqrt(r)+0.01;
  //float rad = glm::radians(45.);
  //float a = r / glm::cos(rad);
  //float b = 2 * glm::tan(rad);
  //return a / sqrt(b);
}

// y(x) = tan() * x - 1/2 * (x/v*cos())^2 + y_0
float HeightAtDistance( const float &dist
                      , const float &vel
                      , const float &angle
                      , const float &y0
                      ) {
  float rad = glm::radians(angle);
  float a = glm::tan(rad) * dist;
  float t = vel*glm::cos(rad) / dist;
  float b = 0.5 / (t*t);
  float c = y0;
  return a - b + c;
}

// h_max = (v*sin())^2 / 2
float MaxHeightReached( const float &vel
                      , const float &angle
                      ) {
  float vs = vel * glm::sin(glm::radians(angle));
  return vs * vs / 2;
}

float MaxAngleToReach( const float &r , const float &y , const float &v ) {
  float v2 = v * v;
  float in = ( v2 + sqrt( v2*v2 - r*r + 2*y*v2 ) ) / r;
  return glm::degrees(glm::atan(in));
}
float MinAngleToReach( const float &r , const float &y , const float &v ) {
  float v2 = v * v;
  float in = ( v2 - sqrt( v2*v2 - r*r + 2*y*v2 ) ) / r;
  return glm::degrees(glm::atan(in));
}

