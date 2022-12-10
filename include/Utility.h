#pragma once

#include "Types.h"

#include <string>

Pos2Df PixelsToGlPos( int x , int y );

Pos2D GlPosToPixels( float x , float y );

std::string JoinPath(const std::string &a, const std::string &b);

std::string StripQuotes(const std::string &in);

void AddSquare2D( const float vx0 , const float vy0
                , const float vx1 , const float vy1
                , std::vector<glm::vec3> &verts
                , std::vector<GLuint> &ids
                );

void AddSquare2D( const float vx0 , const float vy0
                , const float vx1 , const float vy1
                , const float tx0 , const float ty0
                , const float tx1 , const float ty1
                , std::vector<glm::vec3> &verts
                , std::vector<GLuint> &ids
                , std::vector<glm::vec2> &texture
                );

