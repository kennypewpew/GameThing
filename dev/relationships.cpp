#include "Window.h"
#include "GlLayer.h"
#include "GlobalActor.h"
#include "Globals.h"
#include "Utility.h"
#include "GUILayer.h"

#include <unistd.h>

const unsigned int DISP_WIDTH = 960;
const unsigned int DISP_HEIGHT = 960;

const char* solidColorVS = "shader/solidColor.vs";
const char* solidColorFS = "shader/solidColor.fs";
const char* textureVS = "shader/inputTexture.vs";
const char* textureFS = "shader/inputTexture.fs";
const char* coloredTextureFS = "shader/coloredTexture.fs";

class RelationWindow {
 public:
  size_t activeID;

  bool quit;
  bool redraw;
  float zoom;
  glm::mat4 view;

  std::vector<glm::vec3> verCorners;
  std::vector<GLuint>    indCorners;
  std::vector<glm::vec2> texCorners;

  glm::vec3 textColor = glm::vec3(1.0,1.0,1.0);

  Shader textBgShader, textShader;
  TextBackground textBgLayer;
  //TextWriter textWriter;

  RelationWindow() :
        zoom(1.), quit(false), view(glm::mat4(1.f))
      , textBgShader(ShaderBase( solidColorVS, solidColorFS ))
      , textShader(ShaderBase( textureVS, coloredTextureFS ))
      , textBgLayer(textBgShader)
  {
    AddSquare2D( -1, -1, +1, +1, verCorners, indCorners );

    CharMap fontInfo[3];
    fontInfo[0].ReadFontFile("assets/fonts","gidole_regular_10.fnt");
    fontInfo[1].ReadFontFile("assets/fonts","gidole_regular_15.fnt");
    fontInfo[2].ReadFontFile("assets/fonts","gidole_regular_20.fnt");

    //textWriter(textShader,fontInfo[2],textColor,SCREEN.W(),SCREEN.H());
}

  void HandleInputEvent( const SDL_Event &e ) {
      if (e.type == SDL_QUIT) {
        this->quit = true;
      }
      else if ( e.type == SDL_KEYDOWN ) {
        switch (e.key.keysym.sym)
        {
          case SDLK_PERIOD:
            break;
          case SDLK_SPACE:
            break;
          case SDLK_a:
            break;
          case SDLK_d:
            break;
          case SDLK_s:
            break;
          case SDLK_w:
            break;
          case SDLK_UP:
            break;
          case SDLK_DOWN:
            break;
          case SDLK_LEFT:
            break;
          case SDLK_RIGHT:
            break;
          case SDLK_y:
            break;
          case SDLK_t:
            break;
          case SDLK_q:
            this->quit = true;
            break;
          default:
            break;
        }
      }
      else if ( uint8_t(e.type) == uint8_t(SDL_MOUSEBUTTONDOWN) ) {
        switch(e.button.button)
        {
          case SDL_BUTTON_LEFT:
            {
              int xLast, yLast;
              SDL_GetMouseState(&xLast,&yLast);
              break;
            }
          case SDL_BUTTON_RIGHT:
            {
              int xLast, yLast;
              SDL_GetMouseState(&xLast,&yLast);
              break;
            }
          case SDL_BUTTON_MIDDLE:
            break;
          case SDL_BUTTON_X1:
            break;
          case SDL_BUTTON_X2:
            break;
          default:
            break;
        }
      }
      else if ( e.type == SDL_MOUSEBUTTONUP ) {
        switch(e.button.button)
        {
          case SDL_BUTTON_LEFT:
            break;
            case SDL_BUTTON_RIGHT:
            break;
          case SDL_BUTTON_MIDDLE:
            break;
          case SDL_BUTTON_X1:
            break;
          case SDL_BUTTON_X2:
            break;
          default:
            break;
        }
      }
      else if ( e.type == SDL_MOUSEWHEEL ) {
        if (e.wheel.y > 0 || e.mgesture.dDist > 0 ) {
          //Zoom(+0.05f);
        }
        else if (e.wheel.y < 0 ) {
          //Zoom(-0.05f);
        }
      }
      else if ( e.type == SDL_MULTIGESTURE ) {
        if ( e.mgesture.dDist > 0 ) {
          //Zoom( e.mgesture.dDist/500. );
        }
        else if ( e.mgesture.dDist < 0 ) {
          //Zoom( e.mgesture.dDist/500. );
        }
      }
  }


  int MainLoop(int argc, char **argv) {
    //std::string inputTextureVS = shaderFileToString( "shader/inputTexture.vs" );
    //std::string inputTextureFS = shaderFileToString( "shader/inputTexture.fs" );
    //std::string fragmentSourceColorTextureStr = shaderFileToString( "shader/coloredTexture.fs" );


    GlLayer background( textBgShader );
    background.AddInput( "position" , 3 );
    background.SetUniform( "zoom" , glm::mat4(1.) );
    background.SetUniform( "shift" , glm::mat4(1.) );
    background.SetUniform( "incolor" , glm::vec4(0.5,0.5,0.5,0.5) );
    background.BindCopyVB(verCorners,3,0);
    background.BindCopyIB(indCorners);


    while (!this->quit) {
      SDL_Event event;
      while (SDL_PollEvent(&event) > 0) {
        HandleInputEvent( event );
      }

      glClear(GL_COLOR_BUFFER_BIT);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

      background.UseThisLayer();
      background.SetUniform( "view" , this->view );
      background.BindCopyVB( verCorners , 3 );
      background.BindCopyIB( indCorners);
      glDrawElements(GL_TRIANGLES,indCorners.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

      SwapWindows();

      //// This should be a low latency screen
      //usleep(100 * MS);
    }

    return 0;
  }
};

int main(int argc, char **argv) {
  InitializeSDL("Relationship Window",DISP_WIDTH,DISP_HEIGHT);

  RelationWindow main;

  return main.MainLoop(argc, argv);
}

