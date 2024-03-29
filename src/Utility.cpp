#include "Utility.h"
#include "Types.h"
#include "Globals.h"

#include <string>
#include <algorithm>
#include <vector>
#include <functional>

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

Pos2Df PixelsToGlPos( int x , int y ) {
  float xOut = +2. * (float(x) / SCREEN.W()) - 1.;
  float yOut = -2. * (float(y) / SCREEN.H()) + 1.;
  return { .x = xOut , .y = yOut };
}

Pos2D GlPosToPixels( float x , float y ) {
  int xOut =                   (x + 1) * SCREEN.W() / 2.;
  int yOut = SCREEN.H() - int( (y + 1) * SCREEN.H() / 2. );
  return { .x = xOut , .y = yOut };
}

std::string JoinPath(const std::string &a, const std::string &b) {
  return a + "/" + b;
}

std::string StripQuotes(const std::string &in) {
  std::string res(in);
  res.erase(std::remove( res.begin(), res.end(), '\"' ),res.end());
  return res;
}

void SetSquare2DPos( const float vx0 , const float vy0
                   , const float vx1 , const float vy1
                   , std::vector<glm::vec3> &verts
                   , const size_t offset
                   ) {
  size_t initialId = offset;
  verts[initialId + 0] = glm::vec3(vx0,vy0,0.);
  verts[initialId + 1] = glm::vec3(vx0,vy1,0.);
  verts[initialId + 2] = glm::vec3(vx1,vy1,0.);
  verts[initialId + 3] = glm::vec3(vx1,vy0,0.);
}

void AddSquare2D( const float vx0 , const float vy0
                , const float vx1 , const float vy1
                , std::vector<glm::vec3> &verts
                , std::vector<GLuint> &ids
                ) {
  size_t initialId = verts.size();
  verts.push_back(glm::vec3(vx0,vy0,0.));
  verts.push_back(glm::vec3(vx0,vy1,0.));
  verts.push_back(glm::vec3(vx1,vy1,0.));
  verts.push_back(glm::vec3(vx1,vy0,0.));
  ids.push_back(initialId+0);
  ids.push_back(initialId+1);
  ids.push_back(initialId+2);
  ids.push_back(initialId+0);
  ids.push_back(initialId+2);
  ids.push_back(initialId+3);
}

void AddSquare2D( const float vx0 , const float vy0
                , const float vx1 , const float vy1
                , glm::vec3 &color
                , std::vector<glm::vec3> &verts
                , std::vector<GLuint> &ids
                , std::vector<glm::vec3> &colors
                ) {
  AddSquare2D( vx0 , vy0 , vx1 , vy1 , verts , ids );
  for ( int i = 0 ; i < 4 ; ++i ) colors.push_back(color);
}


void AddSquare2D( const float vx0 , const float vy0
                , const float vx1 , const float vy1
                , const float tx0 , const float ty0
                , const float tx1 , const float ty1
                , std::vector<glm::vec3> &verts
                , std::vector<GLuint> &ids
                , std::vector<glm::vec2> &texture
                ) {
  size_t initialId = verts.size();
  verts.push_back(glm::vec3(vx0,vy0,0.));
  verts.push_back(glm::vec3(vx0,vy1,0.));
  verts.push_back(glm::vec3(vx1,vy1,0.));
  verts.push_back(glm::vec3(vx1,vy0,0.));
  ids.push_back(initialId+0);
  ids.push_back(initialId+1);
  ids.push_back(initialId+2);
  ids.push_back(initialId+0);
  ids.push_back(initialId+2);
  ids.push_back(initialId+3);
  texture.push_back(glm::vec2(tx0,ty0));
  texture.push_back(glm::vec2(tx0,ty1));
  texture.push_back(glm::vec2(tx1,ty1));
  texture.push_back(glm::vec2(tx1,ty0));
}

void AddTextureCoords( std::vector<glm::vec2> &texArray , Box2Df tCoords ) {
  texArray.push_back(glm::vec2(tCoords.x0,tCoords.y0));
  texArray.push_back(glm::vec2(tCoords.x0,tCoords.y1));
  texArray.push_back(glm::vec2(tCoords.x1,tCoords.y1));
  texArray.push_back(glm::vec2(tCoords.x1,tCoords.y0));
}

void CreateConnectedGrid( int xDim
                        , int yDim
                        , std::vector<glm::vec3> &vertices
                        , std::vector<GLuint> &indices
                        ) {
  for ( int j = 0 ; j <= yDim ; ++j ) {
    for ( int i = 0 ; i <= xDim ; ++i ) {
      float x = (float(i) / float(xDim) * 2. - 1.)*0.99;
      float y = (float(j) / float(yDim) * 2. - 1.)*0.99;
      vertices.push_back(glm::vec3(x,y,0));
    }
  }
  for ( int j = 0 ; j < yDim ; ++j ) {
    indices.push_back( ( j ) * (xDim+1) );
    indices.push_back( (j+1) * (xDim+1) );

    for ( int i = 0 ; i < xDim ; ++i ) {
      indices.push_back( ( j ) * (xDim+1) + (i  ) );
      indices.push_back( (j+1) * (xDim+1) + (i+1) );

      indices.push_back( ( j ) * (xDim+1) + (i  ) );
      indices.push_back( ( j ) * (xDim+1) + (i+1) );

      indices.push_back( ( j ) * (xDim+1) + (i  ) );
      indices.push_back( (j+1) * (xDim+1) + (i  ) );

      indices.push_back( (j+1) * (xDim+1) + (i+1) );
      indices.push_back( ( j ) * (xDim+1) + (i+1) );

      indices.push_back( (j+1) * (xDim+1) + (i+1) );
      indices.push_back( (j+1) * (xDim+1) + (i  ) );
    }
  }
  for ( int j = 0 ; j < xDim ; ++j ) {
    indices.push_back( (yDim+1)*xDim + j   );
    indices.push_back( (yDim+1)*xDim + j+1 );
  }

}

void CreateConnectedGridColored( int xDim
                               , int yDim
                               , std::vector<glm::vec3> &vertices
                               , std::vector<GLuint> &indices
                               , std::vector<glm::vec3> &colors
                               , std::function<glm::vec3(int,int,int,int)> &f
                               ) {
  CreateConnectedGrid(xDim, yDim, vertices, indices);
  for ( int j = 0 ; j <= yDim ; ++j ) {
    for ( int i = 0 ; i <= xDim ; ++i ) {
      colors.push_back(f(i,j,xDim,yDim));
    }
  }
}

void CreateCyclonePointsColored( float r0
                               , float r1
                               , int degInc
                               , float height
                               , int hLevels
                               , std::vector<glm::vec3> &vertices
                               , std::vector<GLuint> &indices
                               , std::vector<glm::vec3> &colors
                               ) {
  const double pi = std::acos(-1);
  uint32_t cntr = 0;

  // Takes [0,1] input and makes it a nicer shape
  auto radiusFunction = [](float in) {
    return in*in;
  };

  for ( int hInc = 0 ; hInc <= hLevels ; ++hInc ) {
    float hProp = float(hInc) / float(hLevels);
    float r = radiusFunction(hProp) * (r1 - r0) + r0;

    for ( int deg = 0 ; deg < 360 ; deg += degInc ) {
      float angle = float(deg) * pi / 180.;

      float z = height * hProp;

      float R = 0.5 + sin(angle)/2.;
      float G = 0.5 + cos(angle)/2.;
      float B = hProp;

      vertices.push_back(glm::vec3(r,angle,-z));
      colors.push_back(glm::vec3(R,G,B));
      indices.push_back(cntr++);
    }
  }
}

void CreateSpherePointsColored( float r
                              , int degInc
                              , float height
                              , int hLevels
                              , std::vector<glm::vec3> &vertices
                              , std::vector<GLuint> &indices
                              , std::vector<glm::vec3> &colors
                              ) {
  const double pi = std::acos(-1);
  uint32_t cntr = 0;

  for ( int degA = 0 ; degA < 360 ; degA += degInc ) {
    for ( int degB = 0 ; degB < 360 ; degB += degInc ) {
      float theta = float(degA) * pi / 180.;
      float phi   = float(degB) * pi / 180.;

      float R = 0.5 + 0.5*(cos(theta) * sin(phi  ));
      float G = 0.5 + 0.5*(sin(theta) * sin(phi  ));
      float B = 0.5 + 0.5*(             cos(phi  ));

      vertices.push_back(glm::vec3(r,theta,phi));
      colors.push_back(glm::vec3(R,G,B));
      indices.push_back(cntr++);
    }
  }
}

void FpsPrinter::Tick() {
  ++fpsCount;
  uint32_t t = SDL_GetTicks();
  elapsed += t - prevTick;
  prevTick = t;
  if ( elapsed > oneSecond ) {
    printf("%d FPS\n", fpsCount);
    elapsed = elapsed % oneSecond;
    fpsCount = 0;
  }
}

void FrameTimeLimiter::Tick() {
  uint32_t t = SDL_GetTicks();
  elapsed = t - prevTick;
  if ( elapsed < minTime ) {
    uint32_t waitTime = minTime - elapsed;
    usleep( waitTime * 1000 - 200 ); // Account some time for running Tick()
  }
  t = SDL_GetTicks();
  prevTick = t;
}

