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

void shaderCompileCheck(GLint s) {
  GLint status;
  glGetShaderiv(s, GL_COMPILE_STATUS, &status);
  if ( status != GL_TRUE ) {
    char buffer[512];
    glGetShaderInfoLog(s, 512, NULL, buffer);
    printf("%s\n",buffer);
  }
}

  GlLayer::GlLayer( const char *vShader
         , const char *fShader
         ) {
    glGenVertexArraysOES(1,&this->va);
    glBindVertexArrayOES(this->va);

    glGenBuffers(1, &this->ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,this->ib);

    this->sp = GL_INVALID_VALUE;

    this->CompileShaderProgram( vShader , fShader );
  }
  GlLayer::~GlLayer() {}

  void GlLayer::BindVB( const int &iBuf ) {
    glBindBuffer(GL_ARRAY_BUFFER, this->vb[iBuf]);
  }

  void GlLayer::BindCopyIB( std::vector<GLuint> &i , GLenum usage ) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,this->ib);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*i.size(), i.data(), usage);
  }

  void GlLayer::CompileShaderProgram( const char *vShader
                           , const char *fShader
                           ) {
    this->sp = glCreateProgram();

    // Compile shaders
    if ( vShader != NULL ) {
      GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertexShader,1,&vShader,NULL);
      glCompileShader(vertexShader);
      shaderCompileCheck(vertexShader);
      glAttachShader(this->sp, vertexShader);
    }

    if ( fShader != NULL ) {
      GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragmentShader,1,&fShader,NULL);
      glCompileShader(fragmentShader);
      shaderCompileCheck(fragmentShader);
      glAttachShader(this->sp, fragmentShader);
    }

    glLinkProgram(this->sp);
    glUseProgram(this->sp);
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

  void GlLayer::AddTexture( const std::string &textureName
                          , const std::string &textureCoordName
                          , const std::string &asset
                          ) {
    GLuint t;
    glGenTextures(1,&t);
    this->tex.push_back(t);
    glBindTexture(GL_TEXTURE_2D,t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int tWidth, tHeight, tChans;
    std::string tFile = asset;
    unsigned char *tData = stbi_load(tFile.c_str(), &tWidth, &tHeight, &tChans, 0);
    if ( tData ) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tWidth, tHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, tData);
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

  void GlLayer::AddUniform( const std::string &uName ) {
    GLuint u = glGetUniformLocation(this->sp, uName.c_str());
    this->uni.push_back(u);
  }

  void GlLayer::SetView( const std::string &uniformName
                          , const glm::mat4 &modification
                          ) {
      GLint uniTrans = glGetUniformLocation(this->sp, uniformName.c_str());
      glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(modification));
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

  void GlLayer::ZoomView( const std::string &uniformName
               , const glm::mat4 &zoom
               ) {
      GLint uniTrans = glGetUniformLocation(this->sp, uniformName.c_str());
      glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(zoom));
  }

  void GlLayer::ShiftView( const std::string &uniformName
               , const glm::mat4 &shift
               ) {
      GLint uniTrans = glGetUniformLocation(this->sp, uniformName.c_str());
      glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(shift));
  }

  void GlLayer::UseThisLayer() {
    glBindVertexArrayOES(this->va);
    for ( size_t i = 0 ; i < tex.size() ; ++i ) glBindTexture(GL_TEXTURE_2D, tex[i] );
    glUseProgram(this->sp);
  }

