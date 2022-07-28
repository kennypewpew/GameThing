#pragma once

#include "Types.h"

#include <vector>
#include <string>

#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class GlLayer {
 public:
  GLuint va; // Vertex array
  std::vector<GLuint> vb; // Vertex buffer
  GLuint ib; // Vertex array
  GLuint sp; // Shader program
  std::vector<GLuint> uni; // Uniform locations
  std::vector<GLuint> aptr; // Attribute pointers
  std::vector<GLuint> tex; // Texture pointers

  GLenum drawType; // Type of drawing (GL_POINTS, GL_LINE, etc.)

  GlLayer( const char *vShader
         , const char *fShader
         );
  ~GlLayer();

  void BindVB( const int &iBuf );

  // T must still be a glm type
  template<typename GLM_T>
  void BindCopyVB( std::vector<GLM_T> &v , const int &nEle , const int &iBuf = 0 , GLenum usage = GL_STATIC_DRAW ) {
    glBindBuffer(GL_ARRAY_BUFFER, this->vb[iBuf]);
    glVertexAttribPointer(aptr[iBuf], nEle, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, v.size()*sizeof(GLM_T), glm::value_ptr(v[0]), usage);
  }

  void BindCopyIB( std::vector<GLuint> &i , GLenum usage = GL_STATIC_DRAW );

  void CompileShaderProgram( const char *vShader
                           , const char *fShader
                           );

  void AddInput( const std::string &inName , const int &sz );

  void AddTexture( const std::string &inName
                 , const std::string &textureCoordName
                 , const std::string &asset
                 );
  //void AddInputTexture( const std::string &inName , const int &sz );

  void AddUniform( const std::string &uName );

  void SetView( const std::string &uniformName
              , const glm::mat4 &view
              );

  void RotateView( const std::string &uniformName
                 , const std::vector<Rotation> &rotations
                 , const std::vector<Translation> &translations
                 );
  void ZoomView( const std::string &uniformName
               , const glm::mat4 &zoom
               );

  void ShiftView( const std::string &uniformName
               , const glm::mat4 &shift
               );

  void UseThisLayer();

};


