#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>
#include <map>

#include "Globals.h"
#include "Window.h"
#include "MenuBase.h"
#include "GlLayer.h"
#include "Types.h"
#include "Containers.hpp"
#include "Utility.h"
#include "Text.h"
#include "GUILayer.h"

#include "SDL.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

void TestPrintW(int x, int y, ClickType c, void* v) {
  printf("WholeWindow: %d , %d\n", x, y);
}

void TestPrint(int x, int y, ClickType c, void* v) {
  printf("%d , %d\n", x, y);
}

void TestDisplay(void* v) {
  printf("display\n");
}

class TestClass {
 public:
  //typedef TestClassArgs MenuArgs;
  struct MenuArgs {
    int x;
    int y;
    int z;
  };


  void MenuSelectHandler(MenuArgs ma) {
    printf("%d\t%d\t%d\n",ma.x,ma.y,ma.z);
  }
};

int main(void) {
  unsigned int DISP_WIDTH = 1000;
  unsigned int DISP_HEIGHT = 800;
  InitializeSDL("GLES3+SDL2 Tutorial",DISP_WIDTH,DISP_HEIGHT);

  std::string vertexSourceTextureStr   = shaderFileToString( "shader/inputTexture.vs" );
  std::string fragmentSourceTextureStr = shaderFileToString( "shader/inputTexture.fs" );
  std::string fragmentSourceColorTextureStr = shaderFileToString( "shader/coloredTexture.fs" );
  std::string vertexSolidColorStr = shaderFileToString( "shader/solidColor.vs" );
  std::string fragmentSolidColorStr = shaderFileToString( "shader/solidColor.fs" );

  Shader bgShader(vertexSourceTextureStr.c_str(), fragmentSourceTextureStr.c_str());
  Shader textShader(vertexSourceTextureStr.c_str(), fragmentSourceColorTextureStr.c_str());
  Shader textBgShader( vertexSolidColorStr.c_str() , fragmentSolidColorStr.c_str() );

  std::vector<glm::vec3> bgVerts;
  std::vector<GLuint> bgIds;
  std::vector<glm::vec2> bgTextures;

  GlLayer bgLayer(bgShader);
  AddSquare2D( -1, -1, +1, +1, 0, 0, 1, 1, bgVerts, bgIds, bgTextures );
  bgLayer.AddInput( "position" , 3 );
  bgLayer.AddInput( "texCoord" , 2 );
  bgLayer.SetUniform( "zoom" , glm::mat4(1.) );
  bgLayer.SetUniform( "shift" , glm::mat4(1.) );
  bgLayer.SetUniform( "view" , glm::mat4(1.) );
  bgLayer.BindCopyVB(bgVerts,3);
  bgLayer.BindCopyIB(bgIds);
  bgLayer.BindCopyVB(bgTextures,2,1);
  bgLayer.SetTexture("applyTexture","texCoord","assets/dungeonCrawlSoup/dungeon/wall/hive_0.png",GL_RGBA);

  TextBackground textBgLayer(textBgShader);


  CharMap fontInfo[3];
  fontInfo[0].ReadFontFile("assets/fonts","gidole_regular_10.fnt");
  fontInfo[1].ReadFontFile("assets/fonts","gidole_regular_15.fnt");
  fontInfo[2].ReadFontFile("assets/fonts","gidole_regular_20.fnt");

  glm::vec3 textColor(1.0,1.0,1.0);
  TextWriter txtW(textShader,fontInfo[2],textColor,SCREEN.W(),SCREEN.H());

  std::vector<std::string> sequence;
  for ( int i = 1000 ; i < 1010 ; ++i ) sequence.push_back(std::to_string(i));

  MessageSequence m( &txtW , sequence );
  m.AdvanceText();

  ScreenRegion w(0,0,1000,1000);
  w.SetObjDispAndHandle( ClickHandler<MessageSequence> , TestDisplay, &m );
  //ScreenRegion s(0,0,300,300);
  //s.SetObjDispAndHandle( TestPrint , TestDisplay, NULL );
  //uint32_t rId = w.InsertRegion(s);
  //w.RemoveRegion(rId);
  //rId = w.InsertRegion(s);

  w.HandleClick(5,5,LCLICK);

  Pos2Df menuPos = { .x = -0.8 , .y = 0.8 };
  PopupMenu menu(menuPos, &txtW, &textBgLayer, &w, 2.f);

  TestClass tc;
  TestClass::MenuArgs ar;
  ar.x = 0;
  ar.y = 1;
  ar.z = 2;
  MenuSelectHandler<TestClass>(&tc, &ar);

  menu.AddItem( "asdf" );
  menu.AddItem( "fasdfewfe" );
  menu.AddItem( "!?!?!?!" , "" , NULL , NULL , NULL );
  menu.AddItem( "fasdfewfe" );
  menu.AddItem( "asdf" , "" , MenuSelectHandler<TestClass> , &tc, &ar );
  menu.AddItem( "168465456" );

  menu.Display();

  bool quit = false;
  bool menuShown = true;
  while (!quit) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_Event event;
    while (SDL_PollEvent(&event) > 0) {
      if ( event.type == SDL_KEYDOWN ) {
        switch (event.key.keysym.sym)
        {
          case SDLK_q:
            quit = true;
          case SDLK_n:
            m.AdvanceText();
          default:
            continue;
        }
      }
      else if ( event.type == SDL_WINDOWEVENT ) {
        if ( event.window.event == SDL_WINDOWEVENT_RESIZED ) {
          SCREEN.SetDimensions(event.window.data1,event.window.data2);
          glViewport(0,0,event.window.data1,event.window.data2);
          menu.Display();
        }
        break;
      }
      else if ( uint8_t(event.type) == uint8_t(SDL_MOUSEBUTTONDOWN) ) {
        int xPos, yPos;
        SDL_GetMouseState(&xPos,&yPos);
        switch(event.button.button)
        {
          case SDL_BUTTON_LEFT:
            {
              w.HandleClick(xPos,yPos,LCLICK);
              if ( !menuShown ) {
                Pos2Df p = PixelsToGlPos( xPos , yPos );
                menu.bounds = { .x0 = p.x , .y0 = p.y };
                menu.Display();
                menuShown = true;
              }
              break;
            }
          case SDL_BUTTON_RIGHT:
            {
              w.HandleClick(xPos,yPos,RCLICK);
              menu.Remove();
              menuShown = false;
              break;
            }
          default:
            break;
        }
      }
    }
    bgLayer.DrawThisLayer();
    //textBgLayer.DrawThisLayer();
    textBgLayer.Draw();
    //textBgLayer.toDraw.DrawThisLayer();
    //if ( showMenu ) menu.Display();
    txtW.RenderText();

    SwapWindows();
  }

  FinalizeSDL();
}

