#pragma once

#include "Types.h"

#include <vector>
#include <string>

#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

std::string shaderFileToString( const std::string &fl );

class Shader {
 public:
  GLuint sp, vs, fs;

  Shader( const char *vShader
        , const char *fShader
        );
  Shader( const std::string &vShader
        , const std::string &fShader
        );
  ~Shader();
  void Compile( const char *vShader
              , const char *fShader
              );
};

class GlLayer {
 public:
  GLuint va; // Vertex array
  GLuint ib; // Vertex array indices
  GLuint sp; // Shader program
  std::vector<GLuint> vb; // Vertex buffer
  std::vector<GLuint> aptr; // Attribute pointers
  GLuint tex; // Texture pointer
  int idSize;

  std::vector<glm::vec3> vertices;
  std::vector<GLuint> indexes;
  std::vector<glm::vec2> textures;
  bool vChanged, iChanged, tChanged;
  bool usesTexture;

  inline std::vector<glm::vec3>& VertexArray()  { this->vChanged = true; return this->vertices; }
  inline std::vector<GLuint>&    IndexArray()   { this->iChanged = true; return this->indexes ; }
  inline std::vector<glm::vec2>& TextureArray() { this->tChanged = true; return this->textures; }
  inline const std::vector<glm::vec3>& VertexArray()  const { return this->vertices; }
  inline const std::vector<GLuint>&    IndexArray()   const { return this->indexes ; }
  inline const std::vector<glm::vec2>& TextureArray() const { return this->textures; }

  GLenum drawType; // Type of drawing (GL_POINTS, GL_LINE, etc.)

  GlLayer( const Shader &shader );
  ~GlLayer();

  void Construct();

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

  void SetTexture( const std::string &inName
                 , const std::string &textureCoordName
                 , const std::string &asset
                 , const int &format
                 );

  void RotateView( const std::string &uniformName
                 , const std::vector<Rotation> &rotations
                 , const std::vector<Translation> &translations
                 );

  template<typename T>
  void SetUniform( const std::string &name
                 , const T &val
                 );

  void UseThisLayer();
  void DrawThisLayer();

};


