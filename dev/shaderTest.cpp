#include <GL/gl.h>

#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Window.h"
#include "GlLayer.h"
#include "Types.h"
#include "Utility.h"
#include "Visuals.h"

const unsigned int DISP_WIDTH = 720;
const unsigned int DISP_HEIGHT = 720;


class ShowWater {
 public:
  bool quit = false;

  glm::mat4 view;
  glm::mat4 zoom;
  std::vector<glm::vec3> v_grid, v_tiles, v_pieces, v_hl, v_active; // Vertices
  std::vector<GLuint> i_grid, i_tiles, i_pieces, i_hl, i_active; // Incides
  std::vector<glm::vec2> t_pieces; // Textures
  std::vector<glm::vec3> c_tiles, c_pieces; // Colors
  
  uint32_t elapsedTicks = 0;
  bool pause = false;

  void HandleInputEvent( const SDL_Event &e ) {
    if ( e.type == SDL_KEYDOWN ) {
      switch (e.key.keysym.sym)
      {
        case SDLK_q:
          this->quit = true;
          break;
        case SDLK_r:
          this->elapsedTicks = 0;
          break;
        case SDLK_SPACE:
          this->pause = !this->pause;
          break;
        default:
          break;
      }
    }
  }
 
  ShowWater() {}

  int MainLoop(int argc, char **argv) {
    //Shader shaderInputColor( "shader/waterDrop.vs" , "shader/diffuseLight.fs" );
    //int xDim = 500;
    //int yDim = 500;

    //std::function<glm::vec3(int,int,int,int)> colorAlgo = [](int x, int y, int xDim, int yDim) {
    //      float cx = float(x) / float(xDim);
    //      float cy = float(y) / float(yDim);
    //      return glm::vec3(0.,cx,cy);
    //    };
    //CreateConnectedGridColored(xDim, yDim, v_tiles, i_tiles, c_tiles, colorAlgo);

    //Shader shaderInputColor( "shader/cyclone.vs" , "shader/inputColor.fs" );
    Shader shaderInputColor( "shader/whorl.vs" , "shader/inputColor.fs" );
    float r0 = 0.1;
    float r1 = 0.5;
    int degInc = 10;
    float height = 0.5;
    int hLevels = 10;
    CreateCyclonePointsColored( r0 , r1 , degInc , height , hLevels
                              , v_tiles
                              , i_tiles
                              , c_tiles
                              );
    //CreateSpherePointsColored( r1 , degInc , height , hLevels
    //                         , v_tiles
    //                         , i_tiles
    //                         , c_tiles
    //                         );

    glm::mat4 view(1.);
    //view = glm::rotate(view,float(glm::radians(30.)), glm::vec3(0.,0.,1.) );
    view = glm::rotate(view,float(glm::radians(60.)), glm::vec3(1.,0.,0.) );

    GlLayer boardLayer( shaderInputColor );
    boardLayer.AddInput( "position" , 3 );
    boardLayer.AddInput( "incolor" , 3 );
    boardLayer.SetUniform( "zoom" , glm::mat4(1.) );
    boardLayer.SetUniform( "shift" , glm::mat4(1.) );
    boardLayer.SetUniform( "view" , view );
    boardLayer.SetUniform( "center" , glm::vec3(0.,0.,0.) );
    boardLayer.SetUniform( "time" , float(0.) );
    boardLayer.BindCopyVB(this->v_tiles,3,0);
    boardLayer.BindCopyVB(this->c_tiles,3,1);
    boardLayer.BindCopyIB(this->i_tiles);


    uint32_t prev = 0;
    uint32_t repeat = 20000;;

    //int frameCount = 0;
    //uint32_t elapsedSeconds = 0;

    FpsPrinter fps;
    float time = 0;

    while (!this->quit) {
      SDL_Event event;
      while (SDL_PollEvent(&event) > 0) {
        HandleInputEvent( event );
      }

      if ( !this->pause ) {
        uint32_t t = SDL_GetTicks();
        elapsedTicks += t - prev;
        prev = t;
        if ( elapsedTicks > repeat ) elapsedTicks = elapsedTicks % repeat;
        time = float(elapsedTicks) / 1000.;
        time *= 2;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        boardLayer.UseThisLayer();
        boardLayer.SetUniform( "time" , time );

        //glDrawElements(GL_TRIANGLES,this->i_tiles.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
        //glDrawElements(GL_LINES,this->i_tiles.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);
        glDrawElements(GL_POINTS,this->i_tiles.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

        SwapWindows();
      }
      else {
        uint32_t t = SDL_GetTicks();
        prev = t;
      }
      fps.Tick();

    }

    return 0;
  }
};

int main(int argc, char **argv) {
  InitializeSDL("Chess", DISP_WIDTH, DISP_HEIGHT);

  ShowWater water;
  return water.MainLoop(argc, argv);
};
