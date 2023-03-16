#include "GlLayer.h"
#include "Types.h"

#include <vector>
#include <string>
#include <iostream>

#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "SDL.h"


void shaderCompileCheck(GLint s,const char *src) {
  GLint status;
  glGetShaderiv(s, GL_COMPILE_STATUS, &status);
  if ( status != GL_TRUE ) {
    printf("Failed to compile shader:\n");
    char buffer[512];
    glGetShaderInfoLog(s, 512, NULL, buffer);
    printf("%s\n",buffer);
  }
}

std::string shaderFileToString( const char *fl ) {
  SDL_RWops *io = SDL_RWFromFile( fl , "r" );
  if ( io == NULL ) {
    SDL_Log("-----------------------------------");
    SDL_Log("Failed to find shader file: %s",fl);
    return fl;
  }

  // Add a null terminator at the end in case the input isn't null terminated
  size_t sz = SDL_RWsize(io)+1;
  char *buf = (char*)malloc(sz*sizeof(char));
  buf[sz-1] = '\0';
  SDL_RWread(io,buf,sz-1,1);
  SDL_RWclose(io);

  std::string res(buf);
  free(buf);
  return res;
}


ShaderBase::ShaderBase( const char* vShader
                      , const char* fShader
                      ) {
  std::string vString = shaderFileToString( vShader );
  std::string fString = shaderFileToString( fShader );
  this->Compile( vString.c_str() , fString.c_str() );
}

ShaderBase::ShaderBase( const std::string &vShader
                      , const std::string &fShader
                      ) {
  this->Compile( vShader.c_str() , fShader.c_str() );
}

void ShaderBase::Compile( const char *vShader
                        , const char *fShader
                        ) {
  std::vector<ShaderInfo> s = { ShaderInfo(vShader,GL_VERTEX_SHADER), ShaderInfo(fShader,GL_FRAGMENT_SHADER) };
  Compile( s );
}

void ShaderBase::Compile( std::vector<ShaderInfo> &shaders
                        ) {
  this->sp = glCreateProgram();

  // Compile shaders
  for ( size_t i = 0 ; i < shaders.size() ; ++i ) {
    const char *shader = shaders[i].rawTxt;
    if ( shader != NULL ) {
      GLuint s = glCreateShader(shaders[i].type);
      glShaderSource(s,1,&shader,NULL);
      glCompileShader(s);
      shaderCompileCheck(s,shader);
      glAttachShader(this->sp, s);
      shaderIds.push_back(s);
    }
  }

  glLinkProgram(this->sp);
  glUseProgram(this->sp);
}

GLuint Shader::sp() const {
  return ShaderBase::sp;
}

Shader::~Shader() {
  ShaderBase::Destroy();
}

void ShaderBase::Destroy() {
  glDeleteProgram(sp);
  for ( size_t i = 0 ; i < shaderIds.size() ; ++i ) glDeleteShader(shaderIds[i]);
}

void GlLayer::Construct() {
  this->vChanged = false;
  this->iChanged = false;
  this->tChanged = false;
  this->usesTexture = false;
  this->idSize = -1;

  glGenVertexArraysOES(1,&this->va);
  glBindVertexArrayOES(this->va);

  glGenBuffers(1, &this->ib);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,this->ib);
}

GlLayer::GlLayer( const Shader &shader ) {
  this->Construct();
  this->sp = shader.sp();
  glUseProgram(this->sp);
}

GlLayer::~GlLayer() {
  if ( this->usesTexture ) glDeleteTextures(1, &tex);
}

void GlLayer::BindVB( const int &iBuf ) {
  glBindBuffer(GL_ARRAY_BUFFER, this->vb[iBuf]);
}

void GlLayer::BindCopyIB( std::vector<GLuint> &i , GLenum usage ) {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,this->ib);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*i.size(), i.data(), usage);
  this->idSize = i.size();
}

void GlLayer::AddInput( const std::string &inName , const int &sz ) {
  GLuint b;
  glGenBuffers(1, &b);
  this->vb.push_back(b);

  GLuint a = glGetAttribLocation(this->sp,inName.c_str());
  this->aptr.push_back(a);
  if ( a == GLuint(-1) ) {
    printf("Failed to find attribute: %s\n",inName.c_str());
    return;
  }
  glEnableVertexAttribArray(a);
  glVertexAttribPointer(a, sz, GL_FLOAT, GL_FALSE, 0, 0);
}

void GlLayer::SetTexture( const std::string &textureName
                        , const std::string &textureCoordName
                        , const std::string &asset
                        , const int &format
                        ) {
  GLuint t;
  glGenTextures(1,&t);
  this->tex = t;
  this->usesTexture = true;
  glBindTexture(GL_TEXTURE_2D,t);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int tWidth, tHeight, tChans;

  SDL_RWops *tFile = SDL_RWFromFile( asset.c_str() , "r" );
  if ( tFile == NULL ) {
    SDL_Log("-----------------------------------");
    SDL_Log("Failed to find texture file: %s",asset.c_str());
    return;
  }
  size_t sz = SDL_RWsize(tFile);
  unsigned char *buf = (unsigned char*)malloc(sz*sizeof(unsigned char));
  SDL_RWread(tFile,buf,sz,1);
  SDL_RWclose(tFile);
  unsigned char *tData = stbi_load_from_memory(buf, sz, &tWidth, &tHeight, &tChans, 0);
  free(buf);

  if ( tData ) {
    glTexImage2D(GL_TEXTURE_2D, 0, format, tWidth, tHeight, 0, format, GL_UNSIGNED_BYTE, tData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glUniform1i(glGetUniformLocation(this->sp,textureName.c_str()),0);
    GLint texAttrib = glGetAttribLocation(this->sp,textureCoordName.c_str());
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib,2,GL_FLOAT,GL_FALSE,0,0);
    stbi_image_free(tData);
  }
  else {
    std::cout << "Failed to load texture: " << tFile << "\n";
  }
}

void GlLayer::RotateView( const std::string &uniformName
               , const std::vector<Rotation> &rotations
               , const std::vector<Translation> &translations
               ) {
    glm::mat4 view = glm::mat4(1.0f);
    for ( size_t i = 0 ; i < rotations.size() ; ++i ) {
      view = glm::rotate(view,glm::radians(rotations[i].degrees), rotations[i].vector);
    }
    for ( size_t i = 0 ; i < translations.size() ; ++i ) {
      view = glm::translate(view,translations[i].vector);
    }
    GLint uniTrans = glGetUniformLocation(this->sp, uniformName.c_str());
    glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(view));
}

void GlLayer::UseThisLayer() {
  glBindVertexArrayOES(this->va);
  if ( this->usesTexture ) glBindTexture(GL_TEXTURE_2D, tex );
  glUseProgram(this->sp);
}

void GlLayer::DrawThisLayer() {
    this->UseThisLayer();
    glDrawElements(GL_TRIANGLES,this->idSize,GL_UNSIGNED_INT,NULL);
}

// TODO: See if it's worth adding a map to store uniform locations
//         instead of doing lookups each time
#define IMPL_SETUNIFORM_MAT( typeName, glFn )                      \
template<> void GlLayer::SetUniform<typeName>(                     \
          const std::string &name                                  \
        , const typeName &val ) {                                  \
    GLint uniTrans = glGetUniformLocation(this->sp, name.c_str()); \
    if ( uniTrans == -1 )                                          \
      throw std::runtime_error("Failed to find uniform: " + name); \
    assert( uniTrans != -1 );                                      \
    glFn(uniTrans, 1, GL_FALSE, glm::value_ptr(val));              \
}                                                                  \
template void GlLayer::SetUniform( const std::string &name         \
                                 , const typeName &val );

#define IMPL_SETUNIFORM_VEC( typeName, glFn )                      \
template<> void GlLayer::SetUniform<typeName>(                     \
          const std::string &name                                  \
        , const typeName &val ) {                                  \
    GLint uniTrans = glGetUniformLocation(this->sp, name.c_str()); \
    if ( uniTrans == -1 )                                          \
      throw std::runtime_error("Failed to find uniform: " + name); \
    assert( uniTrans != -1 );                                      \
    glFn(uniTrans, 1, glm::value_ptr(val));                        \
}                                                                  \
template void GlLayer::SetUniform( const std::string &name         \
                                 , const typeName &val );

#define IMPL_SETUNIFORM_SCA( typeName, glFn )                      \
template<> void GlLayer::SetUniform<typeName>(                     \
          const std::string &name                                  \
        , const typeName &val ) {                                  \
    GLint uniTrans = glGetUniformLocation(this->sp, name.c_str()); \
    if ( uniTrans == -1 )                                          \
      throw std::runtime_error("Failed to find uniform: " + name); \
    assert( uniTrans != -1 );                                      \
    glFn(uniTrans, val);                                           \
}                                                                  \
template void GlLayer::SetUniform( const std::string &name         \
                                 , const typeName &val );

IMPL_SETUNIFORM_MAT( glm::mat2 , glUniformMatrix2fv )
IMPL_SETUNIFORM_MAT( glm::mat3 , glUniformMatrix3fv )
IMPL_SETUNIFORM_MAT( glm::mat4 , glUniformMatrix4fv )

IMPL_SETUNIFORM_VEC( glm::vec2 , glUniform2fv )
IMPL_SETUNIFORM_VEC( glm::vec3 , glUniform3fv )
IMPL_SETUNIFORM_VEC( glm::vec4 , glUniform4fv )

IMPL_SETUNIFORM_SCA( float  , glUniform1f  )
IMPL_SETUNIFORM_SCA( GLint  , glUniform1i  )
IMPL_SETUNIFORM_SCA( GLuint , glUniform1ui )

#undef IMPL_SETUNIFORM_MAT
#undef IMPL_SETUNIFORM_VEC

