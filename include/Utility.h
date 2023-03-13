#pragma once

#include "Types.h"

#include <string>

Pos2Df PixelsToGlPos( int x , int y );

Pos2D GlPosToPixels( float x , float y );

std::string JoinPath(const std::string &a, const std::string &b);

std::string StripQuotes(const std::string &in);

void SetSquare2DPos( const float vx0 , const float vy0
                   , const float vx1 , const float vy1
                   , std::vector<glm::vec3> &verts
                   , const size_t offset
                   );

void AddSquare2D( const float vx0 , const float vy0
                , const float vx1 , const float vy1
                , std::vector<glm::vec3> &verts
                , std::vector<GLuint> &ids
                );

void AddSquare2D( const float vx0 , const float vy0
                , const float vx1 , const float vy1
                , glm::vec3 &color
                , std::vector<glm::vec3> &verts
                , std::vector<GLuint> &ids
                , std::vector<glm::vec3> &colors
                );

void AddSquare2D( const float vx0 , const float vy0
                , const float vx1 , const float vy1
                , const float tx0 , const float ty0
                , const float tx1 , const float ty1
                , std::vector<glm::vec3> &verts
                , std::vector<GLuint> &ids
                , std::vector<glm::vec2> &texture
                );

void AddTextureCoords( std::vector<glm::vec2> &texArray , Box2Df tCoords );

