#include "Text.h"
#include "Utility.h"
#include "Containers.hpp"
#include "GlLayer.h"
#include "Globals.h"

#include <string>
#include <fstream>
#include <sstream>

CharMap::CharMap() {}

void CharMap::ReadFontFile(const std::string &dirName, const std::string &flName) {
  chars.resize(128);
  std::string line;
  std::ifstream fl( JoinPath(dirName , flName) );
  if ( fl.is_open() ) {
    while ( getline(fl,line) ) {
      std::string word;
      std::istringstream lineStream(line);
      getline(lineStream,word,' ');
      if ( word == "common" ) {
        while ( getline(lineStream,word,' ') ) {
          std::istringstream sep(word);
          std::string field, val;
          getline(sep,field,'=');
          getline(sep,val,'=');
          if ( field == "scaleW" ) this->width = stoi(val);
          if ( field == "scaleH" ) this->height = stoi(val);
          if ( field == "lineHeight" ) this->lineHeight = stoi(val);
        }
      }
      else if ( word == "page" ) {
        while ( getline(lineStream,word,' ') ) {
          std::istringstream sep(word);
          std::string field, val;
          getline(sep,field,'=');
          getline(sep,val,'=');
          if ( field == "file" ) this->pngFileName = JoinPath( dirName , StripQuotes(val) );
        }
      }
      else if ( word == "char" ) {
        int id = -1;
        while ( getline(lineStream,word,' ') ) {
          std::istringstream sep(word);
          std::string field, val;
          getline(sep,field,'=');
          getline(sep,val,'=');
          if ( field == "id" ) id = stoi(val);
          if ( field == "x" )        this->chars[id].x        = stoi(val);
          if ( field == "y" )        this->chars[id].y        = stoi(val);
          if ( field == "width" )    this->chars[id].width    = stoi(val);
          if ( field == "height" )   this->chars[id].height   = stoi(val);
          if ( field == "xoffset" )  this->chars[id].xoffset  = stoi(val);
          if ( field == "yoffset" )  this->chars[id].yoffset  = stoi(val);
          if ( field == "xadvance" ) this->chars[id].xadvance = stoi(val);
        }
      }
    }
    this->chars[' '].x = 0;
    this->chars[' '].y = 0;
    this->chars[' '].width = 0;
    this->chars[' '].height = 0;
    this->chars[' '].xoffset = 0;
    this->chars[' '].yoffset = 0;
    this->chars[' '].xadvance = this->chars['.'].xadvance;
    fl.close();
  }
  else {
    //TODO: Failed to open file
  }
}

float TextLength( const std::string &txt
                , const CharMap &cMap
                , const float &scale
                ) {
  int length = 0;
  for ( auto c : txt ) {
    length += cMap.chars[c].xadvance + 2;
  }
  return float(length) / SCREEN.W();
}

void WriteText( const std::string &txt
              , std::vector<glm::vec3> &posVec
              , std::vector<GLuint> &posIdVec
              , std::vector<glm::vec2> &posTexVec
              , const CharMap &cMap
              , const float &scale
              , const float &xLoc
              , const float &yLoc
              ) {
  int length = 0;
  int width  = SCREEN.W();
  int height = SCREEN.H();
  for ( auto c : txt ) {
    length += cMap.chars[c].xadvance + 2;
  }

  if ( (scale*float(length)/width) + xLoc > 1. ) {
    // TODO: text wrapping goes here?
    printf("Won't fit\n");
  }
  if ( (scale*float(cMap.lineHeight)/height) + yLoc > 1. ) {
    // TODO: text wrapping goes here?
    printf("Won't fit\n");
  }

  float currentPos = xLoc;
  for ( auto c : txt ) {
    const Character &ch = cMap.chars[c];

    float w  = scale * float(ch.width) / float(width);
    float x0 = currentPos;
    float x1 = x0 + w;

    float h  = scale * float(ch.height) / float(height);
    float y0 = yLoc;
    float y1 = yLoc + h;

    currentPos += scale * float(ch.xadvance) / float(width);

    float tLeft = float(ch.x) / cMap.width;
    float tTop = float(ch.y) / cMap.height;
    float tRight = tLeft + (float(ch.width ) / cMap.width );
    float tBot = tTop + (float(ch.height) / cMap.height);

    AddSquare2D( x0, y0, x1, y1, tLeft, tBot, tRight, tTop, posVec, posIdVec, posTexVec);
  }
}

DisplayText::DisplayText( float x
                        , float y
                        , float s
                        , std::string txt
                        ) : xLoc(x)
                          , yLoc(y)
                          , scale(s)
                          , text(txt)
                        {}

DisplayText::DisplayText() {};

TextWriter::TextWriter( Shader &shad
                      , const CharMap &f
                      , const glm::vec3 &c
                      , const int &w
                      , const int &h
                      ) : font(f)
                        , toDraw(shad)
                        , needsToUpdate(false)
                        , color(c)
                        , width(w)
                        , height(h)
                      {
  toDraw.AddInput( "position" , 3 );
  toDraw.AddInput( "texCoord" , 2 );
  toDraw.SetUniform( "zoom" , glm::mat4(1.) );
  toDraw.SetUniform( "shift" , glm::mat4(1.) );
  toDraw.SetUniform( "view" , glm::mat4(1.) );
  toDraw.SetTexture("applyTexture","texCoord",this->font.pngFileName,GL_RGBA);
}

uint32_t TextWriter::AddText( DisplayText t ) {
  this->needsToUpdate = true;
  return this->toWrite.Insert(t);
}

void TextWriter::RemoveText( uint32_t id ) {
  this->needsToUpdate = true;
  this->toWrite.Delete(id);
}

void TextWriter::RenderText() {
  toDraw.UseThisLayer();

  if ( this->needsToUpdate ){
    this->needsToUpdate = false;
    this->posVerts.clear();
    this->texVerts.clear();
    this->posIds.clear();
    for ( auto tw : toWrite.map ) {
      DisplayText &w = tw.second;
      float x = w.xLoc;
      float y = w.yLoc;
      float s = w.scale;
      WriteText( w.text , this->posVerts , this->posIds , this->texVerts , this->font , s , x , y );
    }
    toDraw.SetUniform( "texColor" , this->color );
    toDraw.BindCopyVB(this->posVerts,3);
    toDraw.BindCopyIB(this->posIds);
    toDraw.BindCopyVB(this->texVerts,2,1);
  }

  toDraw.DrawThisLayer();
}

TextBackground::TextBackground(Shader &s) : toDraw(s) {
  toDraw.UseThisLayer();

  toDraw.AddInput( "position" , 3 );
  toDraw.SetUniform( "zoom" , glm::mat4(1.) );
  toDraw.SetUniform( "shift" , glm::mat4(1.) );
  toDraw.SetUniform( "view" , glm::mat4(1.) );
  toDraw.SetUniform( "incolor" , glm::vec4(0.3,0.3,0.3,1.0) );

  toDraw.BindCopyVB(this->verts,3);
  toDraw.BindCopyIB(this->inds);
}

uint32_t TextBackground::AddRectangle( const float vx0 , const float vy0
                     , const float vx1 , const float vy1
                     ) {
  this->changed = true;
  Box2Df b;
  b.x0 = vx0;
  b.x1 = vx1;
  b.y0 = vy0;
  b.y1 = vy1;
  return boxes.Insert(b);
}

void TextBackground::RemoveRectangle( uint32_t id ) {
  this->changed = true;
  boxes.Delete(id);
}

void TextBackground::ConstructBoxes() {
  this->verts.clear();
  this->inds.clear();
  for ( auto box : boxes.map ) {
    auto &b = box.second;
    AddSquare2D( b.x0 , b.y0 , b.x1 , b.y1 , this->verts , this->inds );
  }
}

void TextBackground::Draw() {
  if ( changed ) ConstructBoxes();
  toDraw.UseThisLayer();
  toDraw.BindCopyVB(this->verts,3);
  toDraw.BindCopyIB(this->inds);
  toDraw.DrawThisLayer();
}

