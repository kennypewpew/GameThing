#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "SDL.h"

#include "Window.h"
#include "GlLayer.h"
#include "Types.h"
#include "Utility.h"
#include "Text.h"
#include "GUILayer.h"
#include "Globals.h"

const unsigned int DISP_WIDTH = 720;
const unsigned int DISP_HEIGHT = 720;

const float PI = 3.141529;

class Paddle {
 public:
  float length = 1.;
  float width = 1./5.;
  float pos[2] = {0,0};

  Paddle() {}
  Paddle(float l) : length(l), width(l/5.) {}
  Paddle(float l, float x) : length(l), width(l/5.), pos{x,0} {}

  void Reset() {
    pos[1] = 0.;
  }

  void MoveU(float t) {
    if ( pos[1] <= +1. ) pos[1] += t;
  }

  void MoveD(float t) {
    if ( pos[1] >= -1. ) pos[1] -= t;
  }

  void CreateVerticesAndIndices( std::vector<glm::vec3> &ver, std::vector<GLuint> &ind ) {
    GLuint startInd = ver.size();
    float x1 = -width/2.;
    float x2 = +width/2.;
    float y1 = -length/2.;
    float y2 = +length/2.;
    ver.push_back(glm::vec3(x1,y1,0.));
    ver.push_back(glm::vec3(x2,y1,0.));
    ver.push_back(glm::vec3(x2,y2,0.));
    ver.push_back(glm::vec3(x1,y2,0.));
    ind.push_back(startInd+0);
    ind.push_back(startInd+1);
    ind.push_back(startInd+2);
    ind.push_back(startInd+0);
    ind.push_back(startInd+3);
    ind.push_back(startInd+2);
  }
};

class Ball {
 public:
  float radius = 1.;
  float pos[2] = {0,0};
  float vel[2] = {0,0};
  int nTris = 30;

  Ball() {}
  Ball(float r) : radius(r) {}

  bool CheckCollision(Paddle &p) {
    float distX = abs(pos[0] - p.pos[0]);
    if ( distX > radius ) return false;

    float distY = abs(pos[1] - p.pos[1]);
    if ( distY < (p.length/2.)+radius ) return true;

    return false;
  }

  void Collide(Paddle *pads) {
    if ( vel[0] < 0. ) {
      if ( CheckCollision(pads[0]) ) {
        vel[0] = (pos[0] - pads[0].pos[0])/radius;
        vel[1] = (pos[1] - pads[0].pos[1])/radius/2.;
      }
    }
    else {
      if ( CheckCollision(pads[1]) ) {
        vel[0] = (pos[0] - pads[1].pos[0])/radius;
        vel[1] = (pos[1] - pads[1].pos[1])/radius/2.;
      }
    }
  }

  void Reset(bool goLeft = true) {
    if ( goLeft ) vel[0] = -0.4;
    else          vel[0] = +0.4;
    vel[1] = 0.;

    pos[0] = 0.;
    pos[1] = 0.;
  }

  void Move(float t) {
    for ( int i = 0 ; i < 2 ; ++i )
      pos[i] += t*vel[i];
    if ( pos[1] > +1. ) vel[1] = -vel[1];
    if ( pos[1] < -1. ) vel[1] = -vel[1];
  }

  void CreateVerticesAndIndices( std::vector<glm::vec3> &ver, std::vector<GLuint> &ind ) {
    GLuint startInd = ver.size();
    ver.push_back( glm::vec3(0.,0.,0.) );
    ver.push_back( glm::vec3(0.,radius,0.) );
    for ( int i = 1 ; i <= nTris ; ++i ) {
      float angle = i * ( 2. * PI / nTris );
      float x = radius * sin(angle);
      float y = radius * cos(angle);
      ver.push_back( glm::vec3(x,y,0) );
      ind.push_back(startInd + 0);
      ind.push_back(startInd + i-1);
      ind.push_back(startInd + i);
    }
    ind.push_back(startInd + 0);
    ind.push_back(startInd + 1);
    ind.push_back(startInd + nTris);
  }
};

class PongScreen {
 public:
  bool quit = false;

  Paddle paddle[2];
  Ball ball;

  int score[2] = {0,0};
  int rally = 0;
  bool paused = false;

  glm::mat4 view;
  glm::mat4 zoom;
  std::vector<glm::vec3> v_paddleLf, v_paddleRt, v_ball, v_border;
  std::vector<GLuint> i_paddleLf, i_paddleRt, i_ball, i_border;

  glm::mat4 lShift, rShift, bShift;

  bool moveUp = false;
  bool moveDown = false;

  ScreenRegion w = ScreenRegion(0,0,DISP_WIDTH,DISP_HEIGHT);

  PongScreen() {}

  void PaddleAI(float t) {
    //if ( moveUp   ) paddle[0].MoveU(0.6 * t);
    //if ( moveDown ) paddle[0].MoveD(0.6 * t);

    if ( paddle[0].pos[1] < ball.pos[1] ) paddle[0].MoveU(0.6 * t);
    if ( paddle[0].pos[1] > ball.pos[1] ) paddle[0].MoveD(0.6 * t);
  }

  void CheckIfScored() {
    bool scored = false;

    if ( ball.pos[0] < -1. ) {
      ++score[1];
      scored = true;
    }
    if ( ball.pos[0] > 1. ) {
      ++score[0];
      scored = true;
    }

    if ( scored ) {
      paused = true;
      ResetAll();
    }
  }

  void UpdatePositions(float t) {
    ball.Collide(paddle);
    ball.Move(t);
    if ( moveUp   ) paddle[1].MoveU(0.6 * t);
    if ( moveDown ) paddle[1].MoveD(0.6 * t);

    PaddleAI(t);

    lShift = glm::translate(glm::mat4(1.),glm::vec3(paddle[0].pos[0], paddle[0].pos[1], 0) );
    rShift = glm::translate(glm::mat4(1.),glm::vec3(paddle[1].pos[0], paddle[1].pos[1], 0) );
    bShift = glm::translate(glm::mat4(1.),glm::vec3(     ball.pos[0],      ball.pos[1], 0) );

    CheckIfScored();
  }

  void ResetAll() {
    ball.Reset();
    paddle[0].Reset();
    paddle[1].Reset();
  }

  void HandleInputEvent( const SDL_Event &e ) {
      if ( e.type == SDL_KEYDOWN ) {
        switch (e.key.keysym.sym)
        {
          case SDLK_ESCAPE:
            this->paused = !(this->paused);
            break;
          case SDLK_q:
            this->quit = true;
            break;
          case SDLK_r:
            ResetAll();
            break;
          case SDLK_UP:
            moveUp = true;
            break;
          case SDLK_DOWN:
            moveDown = true;
            break;
          default:
            break;
        }
      }
      if ( e.type == SDL_KEYUP ) {
        switch (e.key.keysym.sym)
        {
          case SDLK_UP:
            moveUp = false;
            break;
          case SDLK_DOWN:
            moveDown = false;
            break;
          default:
            break;
        }
      }
      else if ( uint8_t(e.type) == uint8_t(SDL_MOUSEBUTTONDOWN) ) {
        switch(e.button.button)
        {
          case SDL_BUTTON_LEFT:
            if ( uint8_t(e.type) == uint8_t(SDL_MOUSEBUTTONDOWN) ) {
              int xPos, yPos;
              SDL_GetMouseState(&xPos,&yPos);
              switch(e.button.button)
              {
                case SDL_BUTTON_LEFT:
                  {
                    w.HandleClick(xPos,yPos,LCLICK);
                    break;
                  }
                case SDL_BUTTON_RIGHT:
                  {
                    w.HandleClick(xPos,yPos,RCLICK);
                    break;
                  }
                default:
                  break;
              }
            }
            break;
        }
      }
      else if ( e.type == SDL_MOUSEWHEEL ) {
      }
  }

  void MakeBorders(std::vector<glm::vec3> &v_border, std::vector<GLuint> &i_border) {
    float width = 0.02;
    size_t start = v_border.size();
    v_border.push_back( glm::vec3(-1.      ,-1.      ,0) );
    v_border.push_back( glm::vec3(-1.+width,-1.+width,0) );
    v_border.push_back( glm::vec3(+1.      ,-1.      ,0) );
    v_border.push_back( glm::vec3(+1.-width,-1.+width,0) );
    v_border.push_back( glm::vec3(+1.      ,+1.      ,0) );
    v_border.push_back( glm::vec3(+1.-width,+1.-width,0) );
    v_border.push_back( glm::vec3(-1.      ,+1.      ,0) );
    v_border.push_back( glm::vec3(-1.+width,+1.-width,0) );

    for ( int i = 0 ; i < 4 ; ++i ) {
      i_border.push_back(start + ((0 + 2*i) % 8));
      i_border.push_back(start + ((1 + 2*i) % 8));
      i_border.push_back(start + ((2 + 2*i) % 8));
      i_border.push_back(start + ((1 + 2*i) % 8));
      i_border.push_back(start + ((2 + 2*i) % 8));
      i_border.push_back(start + ((3 + 2*i) % 8));
    }
  }

  int MainLoop(int argc, char **argv) {
    Shader shaderInputColor( "shader/inputColor.vs" , "shader/inputColor.fs" );
    Shader shaderSolidColor( "shader/solidColor.vs" , "shader/solidColor.fs" );


    paddle[0] = Paddle(0.2,-0.9);
    paddle[1] = Paddle(0.2,+0.9);
    ball = Ball(0.03);

    paddle[0].CreateVerticesAndIndices(v_paddleLf,i_paddleLf);
    paddle[1].CreateVerticesAndIndices(v_paddleRt,i_paddleRt);
    ball.CreateVerticesAndIndices(v_ball,i_ball);

    GlLayer l_paddleLf( shaderSolidColor );
    l_paddleLf.AddInput( "position" , 3 );
    l_paddleLf.SetUniform( "incolor" , glm::vec4(1.,1.,1.,1.) );
    l_paddleLf.SetUniform( "zoom" , glm::mat4(1.) );
    l_paddleLf.SetUniform( "shift" , glm::mat4(1.) );
    l_paddleLf.SetUniform( "view" , glm::mat4(1.) );
    l_paddleLf.BindCopyVB(this->v_paddleLf,3,0);
    l_paddleLf.BindCopyIB(this->i_paddleLf);

    GlLayer l_paddleRt( shaderSolidColor );
    l_paddleRt.AddInput( "position" , 3 );
    l_paddleRt.SetUniform( "incolor" , glm::vec4(1.,1.,1.,1.) );
    l_paddleRt.SetUniform( "zoom" , glm::mat4(1.) );
    l_paddleRt.SetUniform( "shift" , glm::mat4(1.) );
    l_paddleRt.SetUniform( "view" , glm::mat4(1.) );
    l_paddleRt.BindCopyVB(this->v_paddleRt,3,0);
    l_paddleRt.BindCopyIB(this->i_paddleRt);

    ball.Reset();
    GlLayer l_ball( shaderSolidColor );
    l_ball.AddInput( "position" , 3 );
    l_ball.SetUniform( "incolor" , glm::vec4(1.,1.,1.,1.) );
    l_ball.SetUniform( "zoom" , glm::mat4(1.) );
    l_ball.SetUniform( "shift" , glm::mat4(1.) );
    l_ball.SetUniform( "view" , glm::mat4(1.) );
    l_ball.BindCopyVB(this->v_ball,3,0);
    l_ball.BindCopyIB(this->i_ball);

    MakeBorders(v_border, i_border);
    GlLayer l_border( shaderSolidColor );
    l_border.AddInput( "position" , 3 );
    l_border.SetUniform( "incolor" , glm::vec4(1.,1.,1.,1.) );
    l_border.SetUniform( "zoom" , glm::mat4(1.) );
    l_border.SetUniform( "shift" , glm::mat4(1.) );
    l_border.SetUniform( "view" , glm::mat4(1.) );
    l_border.BindCopyVB(this->v_border,3,0);
    l_border.BindCopyIB(this->i_border);


    Shader bgShader("shader/inputTexture.vs", "shader/inputTexture.fs");
    Shader textShader("shader/inputTexture.vs", "shader/coloredTexture.fs");
    Shader textBgShader( "shader/solidColor.vs" , "shader/solidColor.fs" );

    TextBackground textBgLayer(textBgShader);
    CharMap fontInfo;
    fontInfo.ReadFontFile("assets/fonts","gidole_regular_20.fnt");
    glm::vec3 textColor(1.0,0.0,0.0);
    TextWriter txtW(textShader,fontInfo,textColor,SCREEN.W(),SCREEN.H());

    Pos2Df menuPos = { .x = -0.1 , .y = 0.10 };
    PopupMenu menu(menuPos, &txtW, &textBgLayer, &w, 2.f);
    menu.AddItem( "dummy" );
    menu.Display();

    FpsPrinter fps;

    int frameTime = 10;
    FrameTimeLimiter limit(frameTime);

    while (!this->quit) {
      SDL_Event event;
      while (SDL_PollEvent(&event) > 0) {
        HandleInputEvent( event );
      }

      if ( !paused ) {
        UpdatePositions(float(frameTime)/1000.);
        if ( menu.displayed ) {
          menu.Remove();
        }
      }
      else {
        if ( !menu.displayed ) {
          menu.items[0].text = std::to_string(score[0]) + " : " + std::to_string(score[1]);
          menu.Display();
        }
      }

      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      l_border.UseThisLayer();
      l_paddleLf.SetUniform( "shift" , glm::mat4(1.) );
      glDrawElements(GL_TRIANGLES,this->i_border.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

      l_paddleLf.UseThisLayer();
      l_paddleLf.SetUniform( "shift" , lShift );
      glDrawElements(GL_TRIANGLES,this->i_paddleLf.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

      l_paddleRt.UseThisLayer();
      l_paddleRt.SetUniform( "shift" , rShift );
      glDrawElements(GL_TRIANGLES,this->i_paddleRt.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

      l_ball.UseThisLayer();
      l_ball.SetUniform( "shift" , bShift );
      glDrawElements(GL_TRIANGLES,this->i_ball.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

      textBgLayer.Draw();
      txtW.RenderText();

      limit.Tick();
      fps.Tick();

      SwapWindows();
    }

    return 0;
  }
};

int main(int argc, char **argv) {
  InitializeSDL("Pong", DISP_WIDTH, DISP_HEIGHT);

  PongScreen pong;
  return pong.MainLoop(argc, argv);
};
